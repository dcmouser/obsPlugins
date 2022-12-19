//---------------------------------------------------------------------------
#include <cstdio>
#include "jrPlugin.h"
//
#include "obs.h"
#include <obs-frontend-api.h>

// needed for access to scene item structure
#include <pthread.h>
#include <obs-scene.h>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool JrPlugin::isThisPluginSourceActiveAndVisible() {
	// see https://obsproject.com/docs/reference-sources.html?highlight=active#c.obs_source_active
	bool detectJustFinalOutput = false;

	if (detectJustFinalOutput) {
		// detect only if in final output
		return obs_source_active(getThisPluginSource());
	}
	// check if just showing anywhere?
	return obs_source_showing(getThisPluginSource());
}



obs_source_t* JrPlugin::findActiveAndVisibleAutoZoomSource() {
	// get current scene from preview or non-studio mode
	bool previewMode = false;
	if (obs_frontend_preview_enabled() && obs_frontend_preview_program_mode_active()) {
		previewMode = true;
	}
	// get current scene.  NOTE: we have to release it before we return
	//obs_scene_t* current_scene = NULL;
	obs_source_t* current_sceneAsSource = NULL;
	if (previewMode) {
		current_sceneAsSource = obs_frontend_get_current_preview_scene();
	}
	else {
		current_sceneAsSource = obs_frontend_get_current_scene();
	}

	if (current_sceneAsSource == NULL) {
		// no current scene found
		return NULL;
	}

	obs_scene_t* currentScene = obs_scene_from_source(current_sceneAsSource);

	// walk the scene looking for visible autozoom objects
	obs_source_t * sourcep = findActiveAndVisibleAutoZoomSourceInSceneUsingEnum(currentScene);
	//obs_source_t * sourcep = findActiveAndVisibleAutoZoomSourceInScene(current_scene);

	// release scene
	obs_source_release((obs_source_t*)current_sceneAsSource);

	// ok item may be filled
	if (sourcep == NULL) {
		// nothing found
		//blog(LOG_WARNING, "Did not find a sibling autozoom source to pass along command to.");
		return NULL;
	}

	// got one
	//const char* sourceName = obs_source_get_name(sourcep);
	//blog(LOG_WARNING, "Found a sibling autozoom source: %s.", sourceName);

	// return found item
	return sourcep;
}



obs_source_t* JrPlugin::findActiveAndVisibleAutoZoomSourceInScene(obs_scene_t* scenep) {
	obs_sceneitem_t *item = scenep->first_item;

	while (item) {
		struct obs_scene_item *next = item->next;
		// add ref?
		obs_sceneitem_addref(item);

		//blog(LOG_WARNING, "Checking an item in scene.");

		// is it visible?
		if (obs_sceneitem_visible(item) || true) {
			// it's visible, is it a source?
			obs_source_t* itemSourcep = obs_sceneitem_get_source(item);
			if (itemSourcep) {
				// it's a source but is it OUR kind of source
				const char* sourceId = obs_source_get_id(itemSourcep);
				if (false) {
					// found it
					// release item before we return
					obs_sceneitem_release(item);
					// return source
					return itemSourcep;
				} else {
					// no good
					//const char* sourceName = obs_source_get_name(itemSourcep);
					//blog(LOG_WARNING, "In findAutoZoomSourceInScene, rejecting sourceId: %s  with sourceName: %s.", sourceId, sourceName);
				}
			} else {
				//blog(LOG_WARNING, "itemSourcep was null, skipping check.");
			}
		}

		// release ref?
		obs_sceneitem_release(item);
		// move to next
		item = next;
	}
	// not found
	return NULL;
}


obs_source_t* JrPlugin::findActiveAndVisibleAutoZoomSourceInSceneUsingEnum(obs_scene_t* scenep) {
	obs_source_t* sourcep = NULL;
	obs_scene_enum_items(scenep, findAutoZoomSourceFromSceneItemEnum, &sourcep);
	return sourcep;
}


// see https://cpp.hotexamples.com/examples/-/-/obs_scene_enum_items/cpp-obs_scene_enum_items-function-examples.html#0xbebcf825f68dfbe064253c4435f349d78b7a296069e8a8f6348062f2b31d3296-5,,23,
bool findAutoZoomSourceFromSceneItemEnum(obs_scene_t *scene, obs_sceneitem_t *item, void *param)
{
	UNUSED_PARAMETER(scene);

	// is item visible?
	if (!obs_sceneitem_visible(item)) {
		return true;
	}
	// is it a source?
	obs_source_t* itemSourcep = obs_sceneitem_get_source(item);
	if (!itemSourcep) {
		return true;
	}

	// is it an autozoom source?
	const char* sourceId = obs_source_get_id(itemSourcep);
	if (!strstr(sourceId, "ObsAutoZoomSource")) {
		// wrong type
		return true;
	}

	// we got a match
	//const char* sourceName = obs_source_get_name(itemSourcep);
	//blog(LOG_WARNING, "In findAutoZoomSourceInScene, found a good sourceId: %s  with sourceName: %s.", sourceId, sourceName);
	//
	obs_source_t** returnSourcepp = reinterpret_cast<obs_source_t**>(param);
	*returnSourcepp = itemSourcep;

	// return false saying we are DONE
	return false;
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
bool JrPlugin::triggerVisibleActionSignal(uint32_t actionSignalKey) {
	// either handle it internally if we are visible, or send the signal out to others
	if (isThisPluginSourceActiveAndVisible()) {
		return receiveVisibleActionSignal(actionSignalKey);
	}
	// send it out
	return sendVisibleActionSignal(actionSignalKey);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool JrPlugin::sendVisibleActionSignal(uint32_t actionSignalKey) {
	// otherwise we need to look for another source in the current scene that we want to send a command to
	obs_source_t* siblingAutoZoomSource = findActiveAndVisibleAutoZoomSource();
	if (!siblingAutoZoomSource) {
		// no AutoZoomSource visible to send it to
		return false;
	}
	// build and send recognizable signal
	struct obs_key_event keyEvent;
	keyEvent.modifiers = DefActionSignalStructPreset_modifiers;
	keyEvent.native_modifiers = DefActionSignalStructPreset_native_modifiers;
	keyEvent.native_scancode = actionSignalKey;
	keyEvent.native_vkey = DefActionSignalStructPreset_native_vkey;
	bool key_up = DefActionSignalStructPreset_keyUp;
	//
	obs_source_send_key_click(siblingAutoZoomSource, &keyEvent, key_up);
	//
	return true;
}


bool JrPlugin::receiveVisibleActionSignal(uint32_t actionSignalKey) {
	if (actionSignalKey == DefActionSignalKeyToggleAutoUpdate) {
		performToggleAutoUpdate();
	} else if (actionSignalKey == DefActionSignalKeyToggleLastGoodMarkers) {
		performToggleLastGoodMarkers();
	} else if (actionSignalKey == DefActionSignalKeyToggleAutoSourceHunting) {
		performToggleAutoSourceHunting();
	} else if (actionSignalKey == DefActionSignalKeyInitiateOneShot) {
		performInitiateOneShot();
	} else if (actionSignalKey == DefActionSignalKeyToggleCropping) {
		performToggleCropping();
	} else if (actionSignalKey == DefActionSignalKeyToggleCropBlurMode) {
		performCycleCropBlurMode();
	} else if (actionSignalKey == DefActionSignalKeyToggleDebugDisplay) {
		performToggleDebugDisplay();
	} else if (actionSignalKey == DefActionSignalKeyCycleForward) {
		performViewCycleAdvance(1);
	} else if (actionSignalKey == DefActionSignalKeyCycleBackward) {
		performViewCycleAdvance(-1);
	} else if (actionSignalKey == DefActionSignalKeyResetView) {
		performResetView();
	} else {
		// unhandled
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::performToggleAutoUpdate() {
	opt_autoTrack = !opt_autoTrack;
	if (opt_autoTrack) {
	}
	else {
		gotoCurrentMarkerlessCoordinates();
	}
	saveVolatileSettings();
}


void JrPlugin::performToggleAutoSourceHunting() {
	saveCurrentViewedSourceAsManualViewOption();
	opt_enableAutoSourceHunting = !opt_enableAutoSourceHunting;
	saveVolatileSettings();
}

void JrPlugin::performInitiateOneShot() {
	initiateOneShot();
}

void JrPlugin::performToggleCropping() {
	if (opt_zcMode == Def_zcMode_OnlyZoom) {
		opt_zcMode = Def_zcMode_ZoomCrop;
	} else {
		opt_zcMode = Def_zcMode_OnlyZoom;
	}
	saveVolatileSettings();
}

void JrPlugin::performCycleCropBlurMode() {
	//mydebug("In performCycleCropBlurMode with current opt_zcCropStyle = %d.", (int)opt_zcCropStyle);
	if (opt_zcCropStyle == Def_zcCropStyles_blackBars) {
		opt_zcCropStyle = Def_zcCropStyle_blur;
	}
	else if (opt_zcCropStyle == Def_zcCropStyle_blur) {
		opt_zcCropStyle = Def_zcCropStyles_blackBars;
	}
	updateCropStyleDrawTechnique();
	saveVolatileSettings();
}

void JrPlugin::performToggleDebugDisplay() {
	opt_debugRegions = !opt_debugRegions;
	saveVolatileSettings();
}


void JrPlugin::performToggleLastGoodMarkers() {
	// if we are already at saved good marker position, toggle to markerless
	// either way turn off auto tracking
	if (opt_autoTrack) {
		opt_autoTrack = false;
	}

	bool atGoodMarkerLocation = stracker.isViewNearSavedMarkerLocation();
	if (atGoodMarkerLocation) {
		gotoCurrentMarkerlessCoordinates();
	}
	else {
		// return to last good marker location
		gotoLastGoodMarkerLocation();
	}
}


void JrPlugin::performResetView() {
	if (true) {
		// we can just use this function which will toggle
		performToggleLastGoodMarkers();
	}
	else {
		if (opt_autoTrack) {
			opt_autoTrack = false;
			gotoCurrentMarkerlessCoordinates();
		}
	}
}
//---------------------------------------------------------------------------


