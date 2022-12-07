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
void  JrPlugin::initiateFade(int startingSourceIndex, int endingSourceIndex) {
	if (opt_fadeDuration < 0.0001f || opt_fadeMode==0) {
		// fade disabled
		return;
	}

	fadeStartingSourceIndex = startingSourceIndex;
	fadeEndingSourceIndex = endingSourceIndex;
	fadePosition = 0.0f;
	fadeStartTime = clock();
	fadeDuration = (clock_t) (opt_fadeDuration * (float)CLOCKS_PER_SEC);
	fadeEndTime = fadeStartTime + fadeDuration;
	// set viewing to the target
	stracker.sourceIndexViewing = endingSourceIndex;
}

void  JrPlugin::cancelFade() {
	fadeStartingSourceIndex = -1;
	fadeEndingSourceIndex = -1;
	fadePosition = 1.0f;
}

bool JrPlugin::updateFadePosition() {
	if (fadeStartingSourceIndex == -1) {
		return false;
	}
	clock_t curtime = clock();
	if (curtime > fadeEndTime) {
		cancelFade();
		return false;
	}
	fadePosition = (float)(fadeEndTime-curtime) / (float)fadeDuration;
	return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::cancelOneShot() {
	oneShotEngaged = false;
}


void JrPlugin::initiateOneShot() {
	// turn off auto updating
	opt_enableAutoUpdate = false;
	// turn off ignore if it is on?
	opt_ignoreMarkers = false;
	saveVolatileSettings();
	// abort any hunting
	stracker.abortHunting();
	// start oneshot
	oneShotEngaged = true;
	oneShotStage = 0;
}


void JrPlugin::updateOneShotStatus(bool isStill) {
	// this is also where oneshot gets turned off
	//mydebug("In updateOneSHot status %d and stage: %d.", (int) oneShotEngaged, oneShotStage);
	if (!isOneShotEngaged()) {
		return;
	}
	//mydebug("In updateOneSHot stage: %d.", oneShotStage);
	if (oneShotStage == 0 || !isStill) {
		// switch into stage 1 which allows hunting for a short period, and reset clock
		// we also do this if we detect some movement before we get to the end
		oneShotEndTime = (clock_t)(clock() + ((float)CLOCKS_PER_SEC/ 1000.0f) * (float)DEF_MsSecsToHuntAfterOneShotSettles);
		oneShotStage = 1;
	} else {
		// beyond stage 0 we check time and turn off
		if (clock() > oneShotEndTime) {
			// turn off tracking for oneshot
			cancelOneShot();
		}
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrPlugin::goToInitialView() {
	// go to initial markerless view

	stracker.travelToMarkerlessDefault();
}
//---------------------------------------------------------------------------
























































//---------------------------------------------------------------------------
void JrPlugin::gotoCurrentMarkerlessCoordinates() {
	markerlessCycleListAdvance(0);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::viewCycleAdvance(int delta) {
	if (getMarkerlessModeIsManualZoom()) {
		doManualZoomInOutByPercent(opt_manualZoomStepSize * delta);
	} else if (getMarkerlessModeIsPresets()) {
		markerlessCycleListAdvance(delta);
	} else {
		// if markerless use is disabled, we cycle sources
		viewingSourceManuallyAdvance(delta);
	}

	// ATTN: test -- memory leak happens in this saveVolatileSettings call?

	// save current view now
	saveVolatileSettings();

	// switch what we see if we are markerless -- the true option here says switch sources to the new source even if normal autohunt is off
	if (true) {
		// travel to markerless locations EVEN if we have a target and markers, we can rehunt to proper location after
		// so if they manually change they will see the effect even if we are zoomed in, so they can see that they tried to set markerless even while markers are present and controlling
		stracker.travelToMarkerlessInitiate(true);
		stracker.goDirectlyToMakersAndDelayHunting();
	}
	else {
		stracker.reTravelToMarkerlessIfMarkerless(true);
	}
}

void JrPlugin::markerlessCycleListAdvance(int delta) {
	opt_markerlessCycleIndex += delta;
	opt_markerlessCycleIndex = opt_markerlessCycleIndex % stracker.markerlessManager.getEntryCount();
	if (opt_markerlessCycleIndex < 0) {
		opt_markerlessCycleIndex += stracker.markerlessManager.getEntryCount();
	}
	mydebug("markerlessCycleListAdvanceDelta: %d mod %d.", opt_markerlessCycleIndex,stracker.markerlessManager.getEntryCount());
}


void JrPlugin::viewingSourceManuallyAdvance(int delta) {
	// this function is used when the markerless options are diabled and hotkey rotates through sources.. very little reason to use this mode
	// cycle through what source we are viewing -- useful to put on a hotkey to test multiple source cycling stuff
	// NOTE that if we are not in non-autoswitch source mode, we may jump out of this new source almost immediately -- it should only really be called when autoswitch sources is off
	stracker.sourceIndexViewing += delta;
	stracker.sourceIndexViewing = stracker.sourceIndexViewing % stracker.sourceCount;
	if (stracker.sourceIndexViewing < 0) {
		stracker.sourceIndexViewing += stracker.sourceCount;
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrPlugin::doManualZoomInOutByPercent(float scalePercent) {

	// tell the source tracker about our new markerless
	int sourceIndex = opt_manualZoomSourceIndex;
	float zoomLevel = opt_manualZoomSourceScale;
	int alignmentMode = opt_manualZoomAlignment;

	// ok now adjust zoom level by a certain percent
	// note that this MIGHT cause us to transition to a wider or closer source if multiple camera source, based on 	float opt_manualZoomTransitions[DefMaxSources];

	// ok adust zoom
	if (scalePercent == 0.0) {
		// nothing
	} else if (scalePercent < 0.0) {
		// attempt to go up and down symmetrically instead of more
		zoomLevel = zoomLevel / (float)(1.0 - scalePercent);
	} else if (scalePercent > 0.0) {
		zoomLevel = zoomLevel + (scalePercent * zoomLevel);
	}

	// might want this configured based on aspect ratio of display -- let user configure it?
	float minZoom = opt_manualZoomMinZoom;

	//mydebug("In doManualZoomInOutByPercent %f.", zoomLevel);
	// note that we are not allowed to go below 1.0
	if (zoomLevel < minZoom) {
		// ok so either we have to pop to a wider source or clamp here
		if (sourceIndex < 1) {
			// clamp
			zoomLevel = minZoom;
		}
		else {
			// drop to wider source
			--sourceIndex;
			// and now init the zoom level
			zoomLevel = opt_manualZoomTransitions[sourceIndex];
			if (zoomLevel <= 0) {
				zoomLevel = 2;
			}
		}
	} else if (scalePercent >= 0.0 && sourceIndex < stracker.getSourceCount()-1) {
		// when zooming in we see if we need to switch to closer camera
		float zoomLimit = opt_manualZoomTransitions[sourceIndex];
		if (zoomLimit <= 0) {
			zoomLimit = 2.0;
		}
		if (zoomLevel > zoomLimit) {
			// move to closer source
			++sourceIndex;
			zoomLevel = 1.0;
		}
	}

	// save new values
	opt_manualZoomSourceIndex = sourceIndex;
	opt_manualZoomSourceScale = zoomLevel;

	// now update store this look
	stracker.updateManualZoomEntry(sourceIndex, zoomLevel, alignmentMode);
}
//---------------------------------------------------------------------------


























//---------------------------------------------------------------------------
bool JrPlugin::isThisPluginSourceActiveAndVisible() {
	// see https://obsproject.com/docs/reference-sources.html?highlight=active#c.obs_source_active
	// do we want to check if just showing anywhere?
	return obs_source_showing(getThisPluginSource());
	// or just in final output?
	//return obs_source_active(getThisPluginSource());
};



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
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
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
void JrPlugin::smartVisibleViewCycleAdvance(int delta) {
	// ATTN: 12/7/22 - new code that attemps to send this zoom in/out to the ACTIVE VISIBLE autoZoom source on current scene
	// this will let us use a single hotkey to trigger some stuff
	// so this hotkey will NOT work if the autozoom is not active and visible
	if (isThisPluginSourceActiveAndVisible()) {
		// ok if we are active and visible in final output, then just run on us
		viewCycleAdvance(delta);
		return;
	}

	// otherwise we need to look for another source in the current scene that we want to send a command to
	obs_source_t* siblingAutoZoomSource = findActiveAndVisibleAutoZoomSource();
	if (siblingAutoZoomSource != NULL) {
		if (delta < 0) {
			obs_source_media_previous(siblingAutoZoomSource);
		}
		else if (delta > 0) {
			obs_source_media_next(siblingAutoZoomSource);
		}
	}
}
//---------------------------------------------------------------------------

