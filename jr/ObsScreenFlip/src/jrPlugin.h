#pragma once

//---------------------------------------------------------------------------
#include <windows.h>
#include <ctime>
//
// obs
#include <obs-module.h>
//
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
//
#include "obsHelpers.h"
//
#include "jrPluginDefs.h"
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// dumb simple data structure
#define DefMaxSceneEntries		30
#define DefMaxSceneNameLen		80
#define DefMaxSceneEntryLineLen	80
#define DefMaxSceneEntryLen		512
struct SplitSceneEntry {
	char sceneName[DefMaxSceneNameLen];
	float splitPos;
	bool splitPosIsAbsolute;
};
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
class JrPlugin {
public:
	// obs pointer
	obs_source_t *context;
protected:
	int sceneEntryCount;
	SplitSceneEntry sceneEntries[DefMaxSceneEntries];
public:
	gs_effect_t *effectMain;
	gs_eparam_t *param_splitPosition;
	gs_eparam_t *param_mulVal;
public:
	bool opt_enabled;
	bool opt_onlyDuringStreamRec;
public:
	obs_source_t* renderSource;
	uint32_t sourceWidth, sourceHeight;
	float sceneSettingSplitPosition;
	SplitSceneEntry* sceneEntry;
public:
	gs_texrender_t *texrender;
public:
	const char* getPluginIdCharp() { return obs_source_get_id(context); }
	// note sure if this is right way to get the source represented by this plugin
	obs_source_t* getThisPluginContextSource() { return context; }
	obs_source_t* getThisPluginParentSource() { return obs_filter_get_parent(context); }
	obs_source_t* getThisPluginTargetSource() { return obs_filter_get_target(context); }
	//
	bool getIsPluginTypeFilter() { return (obs_source_get_type(context) == OBS_SOURCE_TYPE_FILTER); }
public:
	bool init();
	void deInitialize();
protected:
	bool loadEffects();
public:
	obs_properties_t* doPluginAddProperties();
	void updateSettingsOnChange(obs_data_t* settings);
	static void doGetPropertyDefaults(obs_data_t* settings);
public:
	void reRegisterHotkeys();
public:
	void saveVolatileSettings();
public:
	void forceUpdatePluginSettingsOnOptionChange() { ; };
	void doTick() {;};
	void doRender();
	void handleHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key) {;};
public:
	bool isEnabled();
	bool isStreamingOrRecording();
	float lookupSceneSplitPosition(obs_source_t* source);
public:
	bool renderSourceFlip(bool flagRenderCurrentScene, obs_source_t* source);
protected:
	bool renderFlipScreen();
	bool renderFlipScreenMainDisplay();
protected:
	void setupEffect(float splitPosition);
public:
	void registerMainRenderHook();
	void unRegisterMainRenderHook();
	bool doReceiveMainRenderCallback(bool flagRenderCurrentScene, obs_source_t* source, obs_display_t* display);
protected:
	bool parseFilterSettings(const char *filterCharp);
	void calcSettingsForScene(obs_source_t* source);
	//
	SplitSceneEntry* lookupSceneEntryByName(const char* sceneName);
};
//---------------------------------------------------------------------------



