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
protected:
	virtual const char* getPluginName() { return "getPluginName"; };
	virtual const char* getPluginLabel() { return "getPluginLabel"; };
	virtual const char* getPluginConfigFileName() { return "getPluginConfigFileName"; };
	virtual const char* getPluginOptionsLabel() { return "getPluginOptionsLabel"; };
protected:
	virtual void registerCallbacksAndHotkeys() { ; };
	virtual void unregisterCallbacksAndHotkeys() { ; };
	virtual void loadStuff(obs_data_t* settings) { ; };
	virtual void saveStuff(obs_data_t* settings) { ; };
protected:
	void saveHotkey(obs_data_t *settings, const std::string keyName, const size_t& hotkeyref);
	void loadHotkey(obs_data_t *settings, const std::string keyName, size_t& hotkeyref);
	std::string buildHotkeySettingPath(const std::string keyName);
	void registerHotkey(FpObsHotkeyCallbackT fp, void* objp, const std::string keyName, size_t& hotkeyref, const std::string label);
public:
	virtual void handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key) { ; };
	virtual void setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { ; };
public:
	virtual void optionsFinishedChanging() { ; };
	virtual JrPluginOptionsDialog* createNewOptionsDialog() { return NULL; };
protected:
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
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




//---------------------------------------------------------------------------
#define do_log(level, format, ...) blog(level, "[" ## PLUGIN_LABEL ## "] " format, ##__VA_ARGS__)
//
#define warn(format, ...) do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG, format, ##__VA_ARGS__)
#define obserror(format, ...) do_log(LOG_ERROR, format, ##__VA_ARGS__)
//
#define mydebug(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)
//#define mydebug(format, ...) 
//---------------------------------------------------------------------------
