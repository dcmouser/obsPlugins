#pragma once

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>

#include "jrobsplugin_options.hpp"



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
	std::string calcTimestampFilePath(std::string suffix);
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
};
