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
public:
	virtual void registerCallbacksAndHotkeys() {;};
	virtual void unregisterCallbacksAndHotkeys() {;};
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



