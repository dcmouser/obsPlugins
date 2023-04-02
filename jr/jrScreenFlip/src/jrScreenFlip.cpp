/*
ObsScrenFlip - flips left and right halves of screen (configurable where) so that when you read from preview screen
it looks like you are looking off in the other direction.
*/

#include "pluginInfo.hpp"
//
#include "jrScreenFlip.hpp"
#include "jrScreenFlip_options.hpp"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>

#include <string>
#include <vector>





//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
static jrScreenFlip* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load(void) {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	moduleInstance = new jrScreenFlip();
	return true;
}

void obs_module_unload() {
	if (moduleInstanceIsRegisteredAndAutoDeletedByObs) {
		blog(LOG_INFO, "plugin managed by and should be auto deleted by OBS.");
		return;
	}
	blog(LOG_INFO, "plugin unloading");
	//
	if (moduleInstance != NULL) {
		delete moduleInstance;
		moduleInstance = NULL;
	}
	blog(LOG_INFO, "plugin unloaded");
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
jrScreenFlip::jrScreenFlip(): jrObsPlugin() {
	bool success = init();
	if (!success) {
		mydebug("Failed to init.");
	}
}


jrScreenFlip::~jrScreenFlip() {
	deInitialize();
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void jrScreenFlip::registerCallbacksAndHotkeys() {
	// generic front end events
	obs_frontend_add_event_callback(ObsFrontendEvent, this);

	// hotkeys
	registerHotkey(ObsHotkeyCallback, this, "toggleEnable", hotkeyId_toggleEnable, "Toggle flip enable");
	//
	// register main render callback
	registerMainRenderHook();
}

void jrScreenFlip::unregisterCallbacksAndHotkeys() {
	// generic front end events
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);

	// hotkeys
	unRegisterHotkey(hotkeyId_toggleEnable);

	// main hook
	unRegisterMainRenderHook();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void jrScreenFlip::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	jrScreenFlip *pluginp = reinterpret_cast<jrScreenFlip *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}

void jrScreenFlip::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	jrScreenFlip *pluginp = reinterpret_cast<jrScreenFlip *>(data);
	pluginp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrScreenFlip::handleObsFrontendEvent(enum obs_frontend_event event) {
	// let parent handle some cases (important, triggers saving on exit, etc.)
	jrObsPlugin::handleObsFrontendEvent(event);
}


void jrScreenFlip::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key) {
	// let parent handle some cases? there are no base class cases that i know of
	jrObsPlugin::handleObsHotkeyPress(id, key);
	//
	if (id == hotkeyId_toggleEnable) {
		opt_enabled = !opt_enabled;
	}
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
bool jrScreenFlip::init() {
	sceneEntry = NULL;
	sourceWidth = NULL;
	renderSource = NULL;

	// base class stuff
	initialStartup();

	// stuff we add
	bool success = true;
	if (true) {
		obs_enter_graphics();
		// does this really need to be done in graphics context?
		success &= loadEffects();
		obs_leave_graphics();
	}
	// needs to be in
	texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	return success;
}



void jrScreenFlip::deInitialize() {
	if (true) {
		obs_enter_graphics();
		// free stuff in graphics mode?
		if (effectMain) {
			gs_effect_destroy(effectMain);
			effectMain = NULL;
		}
		if (texrender) {
			gs_texrender_destroy(texrender);
			texrender = NULL;
		}

		obs_leave_graphics();
	}

	// base class stuff
	finalShutdown();
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------
void jrScreenFlip::saveStuff(obs_data_t *settings) {
	// hotkeys
	saveHotkey(settings, "toggleEnable", hotkeyId_toggleEnable);
	obs_data_set_string(settings, "entryFilterString", opt_entryFilterString.c_str());
	obs_data_set_bool(settings, "enabled", opt_enabled);
	obs_data_set_bool(settings, "onlyDuringStreamRec", opt_onlyDuringStreamRec);
}


void jrScreenFlip::loadStuff(obs_data_t *settings) {
	loadHotkey(settings, "toggleEnable", hotkeyId_toggleEnable);
	const char* charp = obs_data_get_string(settings, "entryFilterString");
	if (charp != NULL) {
		opt_entryFilterString = std::string(charp);
	} else {
		opt_entryFilterString = "";
	}
	//
	opt_enabled = obs_data_get_bool(settings, "enabled");
	opt_onlyDuringStreamRec = obs_data_get_bool(settings, "onlyDuringStreamRec");

	// parse breaklines
	parseFilterSettings(opt_entryFilterString.c_str());
}

//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
JrPluginOptionsDialog* jrScreenFlip::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}

void jrScreenFlip::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog)); };

void jrScreenFlip::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	optionDialog->setOptionEnabled(opt_enabled);
	optionDialog->setOptionOnlyStreamingrecording(opt_onlyDuringStreamRec);
	optionDialog->setOptionSceneFilterNewlined(opt_entryFilterString);
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
bool jrScreenFlip::loadEffects() {
	char *effectPath = obs_module_file("jrScreenFlip.effect");
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
bool jrScreenFlip::isEnabled() {
	return (opt_enabled && (!opt_onlyDuringStreamRec || isStreamingOrRecording()));
}



bool jrScreenFlip::isStreamingOrRecording() {
	if (obs_frontend_streaming_active()) {
		return true;
	}
	if (obs_frontend_recording_active()) {
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
float jrScreenFlip::lookupSceneSplitPosition(obs_source_t* source) {
	if (source == NULL) {
		// main preview
	}
	return 0.50f;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrScreenFlip::setupEffect(float splitPosition) {
	gs_effect_set_float(param_splitPosition, splitPosition);
	struct vec2 mulVal { 0, 0 };
	mulVal.x = 1.0f;
	mulVal.y = 1.0f;
	gs_effect_set_vec2(param_mulVal, &mulVal);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void jrScreenFlip::calcSettingsForScene(obs_source_t* source) {
	renderSource = source;
	//

	if (!dirtyConfigData && source == lastSplitSource) {
		// don't need to do anything, reuse last calculated values
		return;
	}

	//mydebug("Recalculating settings for scene at pointer %p.", source);

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

	// we have good clear flag so we can reuse cache next time
	lastSplitSource = source;
	dirtyConfigData = false;
}


SplitSceneEntry* jrScreenFlip::lookupSceneEntryByName(const char* sceneName) {
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
bool jrScreenFlip::parseFilterSettings(const char *filterCharp) {
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

	// set flag to make sure we recheck scenes for split locations, etc.,
	setDirtyConfigData(true);
	return true;
}
//---------------------------------------------------------------------------



