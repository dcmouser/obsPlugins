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

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>




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
	if (moduleInstance != NULL) {
		moduleInstance->onModuleUnload();
	}

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


void obs_module_post_load() {
	//blog(LOG_INFO, "plugin in onModulePostLoad");
	if (moduleInstance != NULL) {
		moduleInstance->onModulePostLoad();
	}
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
void jrScreenFlip::onModulePostLoad() {
	setupWebsocketStuff();
}

void jrScreenFlip::onModuleUnload() {
	shutdownWebsocketStuff();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrScreenFlip::registerCallbacksAndHotkeys() {
	// generic front end events
	obs_frontend_add_event_callback(ObsFrontendEvent, this);

	// hotkeys
	registerHotkey(ObsHotkeyCallback, this, "toggleEnable", hotkeyId_toggleEnable, "Toggle flip enable");

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

	shutdownWebsocketStuff();

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
	obs_data_set_bool(settings, "disableIfDsk", opt_disableIfDsk);
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
	opt_disableIfDsk = obs_data_get_bool(settings, "disableIfDsk");
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
	optionDialog->setOptionDisableIfDsk(opt_disableIfDsk);
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
		// there is a split position set, but before we continue we need to check if DSK turns us off
		bool isDisabledForDsk = false;
		if (opt_disableIfDsk) {
			// check if dsk is on
			if (isDskOn()) {
				isDisabledForDsk = true;
			}
		}
		if (!isDisabledForDsk) {
			// absolute or percentage split?
			if (sceneEntry->splitPosIsAbsolute) {
				// specified as absolute position; scale it by width
				sceneSettingSplitPosition = sceneEntry->splitPos / (float)sourceWidth;
				if (sceneSettingSplitPosition < 0.0 || sceneSettingSplitPosition > 1.0f) {
					sceneSettingSplitPosition = 0.0f;
				}
			}
			else {
				sceneSettingSplitPosition = sceneEntry->splitPos;
			}
		}
		else {
			sceneSettingSplitPosition = 0;
		}
	}
	if (sceneSettingSplitPosition > 0) {
		//mydebug("Found entry for %s: %f", sceneName, sceneSettingSplitPosition);
	}

	// we have good clear flag so we can reuse cache next time
	lastSplitSource = source;
	setDirtyConfigData(false);
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
bool jrScreenFlip::parseFilterSettings(const char* filterCharp) {
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




//---------------------------------------------------------------------------
bool jrScreenFlip::isDskOn() {
	// just return cached dsk state
	return dskIsOn;
}


void jrScreenFlip::dskStateUpdate(bool val) {
	// called by downstream keyer websocket call
	if (val != dskIsOn) {
		//mydebug("Setting dsk state internal val to %d.", (int)val);
		dskIsOn = val;
		// set flag to make sure we recheck scenes for split locations, etc.,
		setDirtyConfigData(true);
	}
}


void jrScreenFlip::setupWebsocketStuff() {
	// it will look somethjing like this
	vendor = obs_websocket_register_vendor("jrScreenFlip");
	if (!vendor) {
		warn("Failed to register websockets vendor.");
		return;
	}

	//mydebug("Registering event_request_cb_JrReceiveVendorBroadcastEvent.");

	// register special request to receive event broadcasts from other vendors
	auto event_request_cb_JrReceiveVendorBroadcastEvent = [](obs_data_t* request_data, obs_data_t* response_data, void* thisptr) {
		//mydebug("IN cb event_request_cb_JrReceiveVendorBroadcastEvent.");
		jrScreenFlip* thisp = static_cast<jrScreenFlip*>(thisptr);
		thisp->handleWsVenderBroadcastEmit(request_data);
	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "ReceiveVendorBroadcastEvent", event_request_cb_JrReceiveVendorBroadcastEvent, this)) {
		warn("Failed to register obs - websocket request ReceiveVendorBroadcastEvent");
	}
	
}


void jrScreenFlip::shutdownWebsocketStuff() {
	// ATTN: THIS IS CRASHING OBS -- not sure why dont have life to deal with it now
	if (vendor) {
		// concerned about crashing
		//obs_websocket_vendor_unregister_request(vendor, "ReceiveVendorBroadcastEvent");
		vendor = NULL;
	}
}


void jrScreenFlip::handleWsVenderBroadcastEmit(obs_data_t* request_data) {
	// ok now we are looking for events from downstream keyer
	// these data look like:
	//{
	//eventData:
	//	{dsk_channel: 7, dsk_name : 'DSK 1', new_scene : 'DSK - Big Text', old_scene : 'DSK - Count Down'},
	//eventType: "dsk_scene_changed",	
	//vendorName : "downstream-keyer"
	//}

	const char* jsonstr = obs_data_get_json(request_data);
	//mydebug("handleWsVenderBroadcastEmit with json requestdata: %s.", jsonstr);

	const char* vendorName = obs_data_get_string(request_data, "vendorName");
	//mydebug("IN cb event_request_cb_JrReceiveVendorBroadcastEvent event vendor = %s.", vendorName);
	if (strcmp(vendorName, "downstream-keyer") == 0) {
		// ok its from downstream keyer, so far so good
		const char* eventType = obs_data_get_string(request_data, "eventType");
		if (strcmp(eventType, "dsk_scene_changed") == 0) {
			// its the event we want
			//mydebug("IN cb event_request_cb_JrReceiveVendorBroadcastEvent event type = %s.", eventType);
			// now we need eventData parsed as object (then released)
			obs_data_t* event_data = obs_data_get_obj(request_data, "eventData");
			const char* new_scene = obs_data_get_string(event_data, "new_scene");
			//mydebug("IN cb event_request_cb_JrReceiveVendorBroadcastEvent new_scene = %s.", new_scene);
			bool newDskIsOn = false;
			if (strcmp(new_scene, "") == 0) {
				newDskIsOn = false;
			}
			else {
				newDskIsOn = true;
			}
			// ATTN: NOTE THAT this code is imperfect -- if the user has MULTIPLE downstream keyers that go on or off, this will confuse it
			// in such a case you would want an associative map indexed by the keyer and only turn off if ALL keyers are off
			// dsk state has changed from on to off or vice versa
			if (newDskIsOn != dskIsOn) {
				dskStateUpdate(newDskIsOn);
			}
			// release extracted obj
			obs_data_release(event_data);
		}
	}
}
//---------------------------------------------------------------------------
