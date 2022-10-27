//---------------------------------------------------------------------------
#include <cstdio>
//
#include <../UI/obs-frontend-api/obs-frontend-api.h>
//
#include "obsHelpers.h"
#include "jrPlugin.h"
#include "jrfuncs.h"
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// forward declarations that are extern "C"
#ifdef __cplusplus
extern "C" {
#endif
	void onHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed);
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
obs_properties_t* JrPlugin::doPluginAddProperties() {
	obs_properties_t *props = obs_properties_create();
	obs_properties_t* propgroup;

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "general", "General Settings", OBS_GROUP_NORMAL, propgroup);
	obs_properties_add_bool(propgroup, Setting_Enabled, Setting_Enabled_Text);
	obs_properties_add_bool(propgroup, Setting_OnlyDuringStreamRec, Setting_OnlyDuringStreamRec_Text);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "filter", "Scene filter", OBS_GROUP_NORMAL, propgroup);
	obs_properties_add_text(propgroup, Setting_SceneFilter, Setting_SceneFilter_Text, OBS_TEXT_MULTILINE );

	return props;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrPlugin::doGetPropertyDefaults(obs_data_t *settings) {
	obs_data_set_default_bool(settings, Setting_Enabled, Setting_Enabled_Def);
	obs_data_set_default_bool(settings, Setting_OnlyDuringStreamRec, Setting_OnlyDuringStreamRec_Def);
	obs_data_set_default_string(settings, Setting_SceneFilter, Setting_SceneFilter_Def);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::reRegisterHotkeys() {
	// we attach the hotkeys EITHER to ourselves (if we are a source) or the sources if we are a filter
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
bool JrPlugin::loadEffects() {
	char *effectPath = obs_module_file("ObsScreenFlip.effect");
	effectMain = gs_effect_create_from_file(effectPath, NULL);
	if (effectMain) {
		// effect .effect file uniform parameters
		param_splitPosition = gs_effect_get_param_by_name(effectMain, "splitPosition");
		param_mulVal = gs_effect_get_param_by_name(effectMain, "mulVal");
	}
	bfree(effectPath);

	if (!effectMain) {
		mydebug("ERROR loading main effect.");
		return false;
	}
	if (!param_splitPosition || !param_mulVal) {
		mydebug("ERROR getting effect params.");
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------




























//---------------------------------------------------------------------------
void JrPlugin::updateSettingsOnChange(obs_data_t *settings) {
	// get options
	opt_enabled = obs_data_get_bool(settings, Setting_Enabled);
	opt_onlyDuringStreamRec = obs_data_get_bool(settings, Setting_OnlyDuringStreamRec);

	const char* sceneFilterCharp = obs_data_get_string(settings, Setting_SceneFilter);
	// ATTN: TODO - parse scene filter
	parseFilterSettings(sceneFilterCharp);


	// and now make changes based on options changing
	forceUpdatePluginSettingsOnOptionChange();
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void JrPlugin::saveVolatileSettings() {
	// called when we change a setting in runtime object that we want to push into the stored configured object in obs
	// these are mostly things that can be toggled with hotkeys
	// ATTN:TODO - decide whether you really DO want to push these to be remembered or reset them?
	// see https://obsproject.com/docs/reference-sources.html

	// borrow settings
	obs_data_t* settings = obs_source_get_settings(getThisPluginContextSource());
	

	// now push the saved properties
	obs_properties_t* props = obs_source_properties(getThisPluginContextSource());
	obs_properties_apply_settings(props, settings);


	// release settings
	obs_data_release(settings);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
bool JrPlugin::init() {
	sceneEntry = NULL;
	sourceWidth = NULL;
	renderSource = NULL;

	bool success = true;
	if (true) {
		obs_enter_graphics();
		// does this really need to be done in graphics context?
		success &= loadEffects();
		obs_leave_graphics();
	}
	// needs to be in
	texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	// register main render callback?
	registerMainRenderHook();

	return success;
}



void JrPlugin::deInitialize() {
	unRegisterMainRenderHook();

	if (true) {
		obs_enter_graphics();
		// free stuff in graphics mode
		obs_leave_graphics();
	}
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
bool JrPlugin::isEnabled() {
	return (opt_enabled && (!opt_onlyDuringStreamRec || isStreamingOrRecording()));
}



bool JrPlugin::isStreamingOrRecording() {
	bool isStreaming = obs_frontend_streaming_active();
	bool isRecording = obs_frontend_recording_active();
	return (isStreaming || isRecording);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
float JrPlugin::lookupSceneSplitPosition(obs_source_t* source) {
	if (source == NULL) {
		// main preview
	}
	return 0.50f;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrPlugin::setupEffect(float splitPosition) {
	gs_effect_set_float(param_splitPosition, splitPosition);
	struct vec2 mulVal;
	mulVal.x = 1.0f;
	mulVal.y = 1.0f;
	gs_effect_set_vec2(param_mulVal, &mulVal);
}
//---------------------------------------------------------------------------





























//---------------------------------------------------------------------------
void JrPlugin::calcSettingsForScene(obs_source_t* source) {
	renderSource = source;
	//
	sourceWidth = obs_source_get_base_width(source);
	sourceHeight = obs_source_get_base_height(source);
	//
	// get scene name
	const char* sceneName = obs_source_get_name(source);
	// look it up
	SplitSceneEntry* sceneEntry = lookupSceneEntryByName(sceneName);
	if (sceneEntry == NULL) {
		// not found
		sceneSettingSplitPosition = 0;
	}
	else {
		if (sceneEntry->splitPosIsAbsolute) {
			// specified as absolute position; scale it by width
			sceneSettingSplitPosition = sceneEntry->splitPos / (float)sourceWidth;
			if (sceneSettingSplitPosition < 0.0 || sceneSettingSplitPosition > 1.0f) {
				sceneSettingSplitPosition = 0.0f;
			}
		} else {
			sceneSettingSplitPosition = sceneEntry->splitPos;
		}
	}
	if (sceneSettingSplitPosition > 0) {
		//mydebug("Found entry for %s: %f", sceneName, sceneSettingSplitPosition);
	}
}


SplitSceneEntry* JrPlugin::lookupSceneEntryByName(const char* sceneName) {
	SplitSceneEntry* sp;
	for (int i = 0; i < sceneEntryCount; ++i) {
		sp = &sceneEntries[i];
		if (strcmp(sp->sceneName, sceneName) == 0) {
			return sp;
		}
	}
	// not found
	return NULL;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool JrPlugin::parseFilterSettings(const char *filterCharp) {
	sceneEntryCount = 0;

	char settingsBuf[DefMaxSceneEntryLen];
	char entrybuf[DefMaxSceneEntryLineLen];
	char splitPosBuf[24];
	float splitPos;
	bool flagSplitPosAbsolute;
	//
	strncpy(settingsBuf, filterCharp, DefMaxSceneEntryLen);
	while (jrLeftCharpSplit(settingsBuf, entrybuf, '\n')) {
		char sceneName[DefMaxSceneNameLen];
		if (jrLeftCharpSplit(entrybuf, sceneName, '|')) {
			strncpy(splitPosBuf, entrybuf, 23);
			splitPosBuf[23] = '\0';
			if (strlen(splitPosBuf) > 0) {
				flagSplitPosAbsolute = true;
				if (splitPosBuf[strlen(splitPosBuf) - 1] == '%') {
					splitPosBuf[strlen(splitPosBuf) - 1] = '\0';
					flagSplitPosAbsolute = false;
				}
				// get split post
				splitPos = (float)atof(splitPosBuf);
				if (!flagSplitPosAbsolute) {
					splitPos = splitPos / 100.0f;
					if (splitPos < 0.0 || splitPos > 1.0f) {
						splitPos = 0;
					}
				}
				// save entry
				SplitSceneEntry* sp = &sceneEntries[sceneEntryCount];
				strcpy(sp->sceneName, sceneName);
				sp->splitPos = splitPos;
				sp->splitPosIsAbsolute = flagSplitPosAbsolute;
				//mydebug("entry for %s is %f (%s)", sceneName, splitPos, splitPosBuf);
				// advance
				++sceneEntryCount;
			}
		}
	}

	return true;
}
//---------------------------------------------------------------------------
