
#pragma once

#include "plugininfo.hpp"
#include "../../jrcommon/src/jrobsplugin.hpp"

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>
#include <obs-module.h>







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
// settings
#define Setting_Enabled						"enabled"
#define Setting_Enabled_Text				obs_module_text("Enable flip screen functionality")
#define Setting_Enabled_Def					true
#define Setting_OnlyDuringStreamRec			"onlyStreamRec"
#define Setting_OnlyDuringStreamRec_Text	obs_module_text("Engage only while streaming or recording")
#define Setting_OnlyDuringStreamRec_Def		false
#define Setting_SceneFilter					"sceneFilter"
#define Setting_SceneFilter_Text			obs_module_text("Scene filter criteria (sceneName | horizontalSplitPercentage%)")
#define Setting_SceneFilter_Def				"MainScene,50%\nSecondaryScene,30%"
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class OptionsDialog;
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
class jrScreenFlip : jrObsPlugin {
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
	std::string opt_entryFilterString;
protected:
	size_t hotkeyId_toggleEnable = -1;
protected:
	bool dirtyConfigData = true;
	obs_source_t* lastSplitSource = NULL;
public:
	obs_source_t* renderSource;
	uint32_t sourceWidth, sourceHeight;
	float sceneSettingSplitPosition;
	SplitSceneEntry* sceneEntry;
public:
	gs_texrender_t *texrender;

public:
	jrScreenFlip();
	virtual ~jrScreenFlip();
protected:
	bool init();
	void deInitialize();
	bool loadEffects();
protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };
protected:
	virtual void loadStuff(obs_data_t *settings);
	virtual void saveStuff(obs_data_t *settings);
	virtual JrPluginOptionsDialog* createNewOptionsDialog();
	virtual void setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog);
public:
	// these all work together
	virtual void registerCallbacksAndHotkeys();
	virtual void unregisterCallbacksAndHotkeys();
	static void ObsFrontendEvent(enum obs_frontend_event event, void* ptr);
	static void ObsHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed);
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
	virtual void handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key);
public:
	virtual void optionsFinishedChanging() { forceUpdatePluginSettingsOnOptionChange(); };
	void setDerivedSettingsOnOptionsDialog(OptionsDialog *optionDialog);
	void forceUpdatePluginSettingsOnOptionChange() {;};
public:
	bool isEnabled();
	bool isStreamingOrRecording();
	float lookupSceneSplitPosition(obs_source_t* source);
public:
	//void doRender();
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
	SplitSceneEntry* lookupSceneEntryByName(const char* sceneName);
	void setDirtyConfigData(bool val) { dirtyConfigData = val; }
public:
	// receiving callbacks from options dialog
	void setOptionEnabled(bool val) { opt_enabled = val; };
	void setOptionOnlyStreamingrecording(bool val)  { opt_onlyDuringStreamRec = val; }
	void setOptionSceneFilterNewlined(std::string str) { opt_entryFilterString = str;  parseFilterSettings(opt_entryFilterString.c_str()); }
};
