#include <obs-module.h>

#include "jrobsplugin.hpp"











//---------------------------------------------------------------------------
const char* jrObsPlugin::getCurrentOutputPath() {
	// based on code from window-basic-main.cpp
	// see also windows-basic-main-screenshot.cpp

	if (mainConfig == NULL) {
		blog(LOG_WARNING, "mainConfig is NULL.");
		return NULL;
	}

	const char *path = nullptr;
	const char *mode = config_get_string(mainConfig, "Output", "Mode");

	if (mode != NULL && strcmp(mode, "Advanced") == 0) {
		const char *advanced_mode =
			config_get_string(mainConfig, "AdvOut", "RecType");

		if (strcmp(advanced_mode, "FFmpeg") == 0) {
			path = config_get_string(mainConfig, "AdvOut",
						 "FFFilePath");
		} else {
			path = config_get_string(mainConfig, "AdvOut",
						 "RecFilePath");
		}
	} else {
		path = config_get_string(mainConfig, "SimpleOutput", "FilePath");
	}
	return path;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
config_t*  jrObsPlugin::getMainConfig() {
	const auto main_window = static_cast<OBSMainWindow *>(obs_frontend_get_main_window());
	config_t* cfg = main_window->Config();
	return cfg;
}

config_t*  jrObsPlugin::getGlobalConfig() {
	config_t* cfg = obs_frontend_get_global_config();
	return cfg;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrObsPlugin::onObsFinishedLoading() {
	flagObsIsFullyLoaded = true;
	regetMainConfigIfNeeded();
}

void jrObsPlugin::regetMainConfigIfNeeded() {
	if (mainConfig == NULL) {
		mainConfig = getMainConfig();
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// from https://github.com/Davidj361/OBS-ChapterMarker/blob/master/src/plugin-main.cpp
void jrObsPlugin::saveSettings() {
	if (!os_file_exists(obs_module_config_path("")))
		createSettingsDir();
	obs_data_t* obj = obs_data_create();
	saveStuff(obj);
	char* file = obs_module_config_path(getPluginConfigFileName());
	obs_data_save_json(obj, file);
	obs_data_release(obj);
}

void jrObsPlugin::loadSettings() {
	char* file = obs_module_config_path(getPluginConfigFileName());
	obs_data_t* obj = obs_data_create_from_json_file(file);
	loadStuff(obj);
}

void jrObsPlugin::createSettingsDir() {
	char* file = obs_module_config_path("");
	os_mkdir(file);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void jrObsPlugin::handleObsFrontendEvent(enum obs_frontend_event event) {
	switch ((int)event) {
		case OBS_FRONTEND_EVENT_FINISHED_LOADING:
			onObsFinishedLoading();
			break;
		case OBS_FRONTEND_EVENT_EXIT:
			onObsExiting();
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void jrObsPlugin::onObsExiting() {
	saveSettings();
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrObsPlugin::initialStartup() {
	regetMainConfigIfNeeded();
	//
	registerCallbacksAndHotkeys();
	//
	setupOptionsDialog();
	// this load of settings must get called AFTER we register hotkeys
	loadSettings();
}

void jrObsPlugin::finalShutdown() {
	unregisterCallbacksAndHotkeys();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// based on exeldro's transition table plugin
//
void jrObsPlugin::setupOptionsDialog() {
	auto *action = (QAction *)obs_frontend_add_tools_menu_qaction(obs_module_text(getPluginOptionsLabel()));

	auto cb = [this] {
		obs_frontend_push_ui_translation(obs_module_get_string);
		JrPluginOptionsDialog* ttd = createNewOptionsDialog();
		ttd->setAttribute(Qt::WA_DeleteOnClose);
		setSettingsOnOptionsDialog(ttd);
		ttd->show();
		obs_frontend_pop_ui_translation();
	};

	QAction::connect(action, &QAction::triggered, cb);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
std::string jrObsPlugin::calcTimestampFilePath(std::string prefix, std::string suffix) {
	// ATTN: TODO - open file
	// see https://github.com/upgradeQ/OBS-Studio-Python-Scripting-Cheatsheet-obspython-Examples-of-API#get-current-profile-settings-via-ffi
	// see https://github.com/obsproject/obs-studio/blob/master/UI/window-basic-main-outputs.cpp
	const char* recPath = getCurrentOutputPath();
	if (recPath == NULL) {
		blog(LOG_WARNING, "Null returned from request for output path.");
		return "";
	}
	std::string recordPathStr = std::string(recPath);
	// fix up record path
	if (recordPathStr == "") {
		// error?
	} else {
		// add / to end
		int slen = (int)recordPathStr.length();
		if (recordPathStr[slen - 1] != '/' && recordPathStr[slen - 1] != '\\') {
			recordPathStr += "/";
		}
	}
	//
	char timestr[255];
	time_t temp;
	struct tm *timeptr;
	temp = time(NULL);
	timeptr = localtime(&temp);
	strftime(timestr, 254, "%Y-%m-%d %H-%M-%S", timeptr);
	//
	std::string filePath = recordPathStr + prefix + std::string(timestr) + suffix;
	//
	return filePath;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void jrObsPlugin::saveHotkey(obs_data_t *settings, const std::string keyName, const size_t& hotkeyref) {
	obs_data_array_t *hotkeys = obs_hotkey_save(hotkeyref);
	std::string fullKeyname = buildHotkeySettingPath(keyName);
	obs_data_set_array(settings, fullKeyname.c_str(), hotkeys);
	obs_data_array_release(hotkeys);
}

void jrObsPlugin::loadHotkey(obs_data_t *settings, const std::string keyName, size_t& hotkeyref) {
	std::string fullKeyname = buildHotkeySettingPath(keyName);
	obs_data_array_t *hotkeys = obs_data_get_array(settings, fullKeyname.c_str());
	if (obs_data_array_count(hotkeys) && hotkeyref != -1) {
		obs_hotkey_load(hotkeyref, hotkeys);
	}
	obs_data_array_release(hotkeys);
}

std::string jrObsPlugin::buildHotkeySettingPath(const std::string keyName) {
	return std::string(getPluginName()) + std::string(".hotkey.") + keyName;
}


void jrObsPlugin::registerHotkey(FpObsHotkeyCallbackT fp, void* objp, const std::string keyName, size_t& hotkeyref, const std::string label) {
	std::string fullKeyname = buildHotkeySettingPath(keyName);
	std::string fullLabel = std::string(getPluginLabel()) + ": " + label;
	if (hotkeyref==-1) hotkeyref = obs_hotkey_register_frontend(fullKeyname.c_str(), fullLabel.c_str(), fp, objp);
}
//---------------------------------------------------------------------------
