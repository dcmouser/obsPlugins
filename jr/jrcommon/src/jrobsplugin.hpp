#pragma once

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>

#include "jrobsplugin_options.hpp"



typedef void (*FpObsHotkeyCallbackT)(void* , obs_hotkey_id id, obs_hotkey_t* , bool );



class jrObsPlugin {
protected:
	config_t* mainConfig = NULL;
	bool flagObsIsFullyLoaded = false;
	bool firstUpdate = true;
protected:
	config_t* getMainConfig();
	config_t* getGlobalConfig();
	const char* getCurrentOutputPath();
	void onObsFinishedLoading();
	void regetMainConfigIfNeeded();
protected:
	std::string calcTimestampFilePath(std::string prefix, std::string suffix);
protected:
	void saveSettings();
	void loadSettings();
	void createSettingsDir();


// ATTN: for SOURCES, which we are not
/*
public:
	// obs pointer should be initialized on creation of a source item
	obs_source_t *context = NULL;
public:
	const char* getPluginIdCharp() { return obs_source_get_id(context); }
	// note sure if this is right way to get the source represented by this plugin
	obs_source_t* getThisPluginContextSource() { return context; }
	obs_source_t* getThisPluginParentSource() { return obs_filter_get_parent(context); }
	obs_source_t* getThisPluginTargetSource() { return obs_filter_get_target(context); }
	bool getIsPluginTypeFilter() { return (obs_source_get_type(context) == OBS_SOURCE_TYPE_FILTER); }
*/

protected:
	virtual const char* getPluginName() { return "getPluginName"; };
	virtual const char* getPluginLabel() { return "getPluginLabel"; };
	virtual const char* getPluginConfigFileName() { return "getPluginConfigFileName"; };
	virtual const char* getPluginOptionsLabel() { return "getPluginOptionsLabel"; };
public:
	virtual void registerCallbacksAndHotkeys() =0; // these MUST be overridden
	virtual void unregisterCallbacksAndHotkeys() =0;
	//
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
	virtual void handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key) { ; };
public:
	virtual void loadStuff(obs_data_t* settings) { ; };
	virtual void saveStuff(obs_data_t* settings) { ; };
protected:
	void saveHotkey(obs_data_t *settings, const std::string keyName, const size_t& hotkeyref);
	void loadHotkey(obs_data_t *settings, const std::string keyName, size_t& hotkeyref);
	std::string buildHotkeySettingPath(const std::string keyName);
	void registerHotkey(FpObsHotkeyCallbackT fp, void* objp, const std::string keyName, size_t& hotkeyref, const std::string label);
	void unRegisterHotkey(size_t& hotkeyid);
public:
	virtual void optionsFinishedChanging() { ; };
	virtual JrPluginOptionsDialog* createNewOptionsDialog() { return NULL; };
	virtual void setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { ; };
protected:
	virtual void onObsExiting();
	virtual void setupOptionsDialog();
public:
	void initialStartup();
	void finalShutdown();
public:
	virtual void postStartup() { ; }
	virtual void initialShutdown() { ; }
public:
	virtual void onModulePostLoad() { ; }
	virtual void onModuleUnload() { ; }
};



