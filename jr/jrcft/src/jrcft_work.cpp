#include "pluginInfo.hpp"
//
#include "jrcft.hpp"
#include "jrcft_options.hpp"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"
//


#include <obs-module.h>
#include <obs.h>
#include <string>

#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>
#include <QObject>


#include <csignal>
#include <signal.h>


#include <windows.h>
#include <shellapi.h>


#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>


// no good because browser sources are plugins not core obs
//struct BrowserSource;
//extern void DispatchJSEvent(std::string eventName, std::string jsonString, BrowserSource *browser = nullptr);


//---------------------------------------------------------------------------
void JrCft::requestWsHandleCommandByClient(obs_data_t *request_data, obs_data_t* response_data) {
	obs_data_set_string(response_data, "ack", "command received by JrCft plugin.");
	//
	const char *comStr = obs_data_get_string(request_data, "command");
	if (!comStr) {
		obs_data_set_string(response_data, "error", "Missing value for 'command' in message to vendor (JrCft).");
		return;
	}
	QString commandString = QString(comStr);

	// handle various commands
	if (commandString == "toggleSourceVisibility") {
		QString sourceName = obs_data_get_string(request_data, "sourceName");
		bool optionAllScenes = obs_data_get_bool(request_data, "optionAllScenes");
		setSourceVisiblityByName(optionAllScenes, sourceName.toStdString().c_str(), JrForceSourceStateToggle);
		obs_data_set_string(response_data, "result", "ok");
	} else if (commandString == "fixAudioMonitoringInScene") {
		fixAudioMonitoringInScenes(false);
		obs_data_set_string(response_data, "result", "ok");
	} else if (commandString == "refreshCurrentSceneBrowserSources") {
		refreshBrowserSourcesInScenes(false);
		obs_data_set_string(response_data, "result", "ok");
	} else if (commandString == "restartMediaInScene") {
		restartMediaSourcesInScenes(false);
		obs_data_set_string(response_data, "result", "ok");
	} else if (commandString == "forwardCustomEvent") {
		// obs_websocket_vendor_emit_event(obs_websocket_vendor vendor, const char *event_name, obs_data_t *event_data)
		mydebug("Triggerred vendor emit event.");
		obs_data_set_string(response_data, "result", "ok");
	} else {
		mydebug("Unknown requestWsHandleCommandByClient: %s.", comStr);
		obs_data_set_string(response_data, "error", "Unknown command issued to vendor (JrCft).");
	}
}


void JrCft::testHotkeyTriggerAction() {
	setSourceVisiblityByName(true, "Overlay - Front Inset", JrForceSourceStateToggle);
}
//---------------------------------------------------------------------------

























//---------------------------------------------------------------------------
// two functions that i previously implemented using standalone scripts
void JrCft::fixAudioMonitoringInScenes(bool flagAllScenes) {
	// callback used on each scene item
	auto cb = [](obs_scene_t*, obs_sceneitem_t* sceneItem, void* param) {
		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
		fixAudioMonitoringInObsSource(itemSource);
		return true;
	};
	doRunObsCallbackOnScenes(flagAllScenes, cb, NULL, false, !flagAllScenes);
}


void JrCft::refreshBrowserSourcesInScenes(bool flagAllScenes) {
	// callback used on each scene item
	auto cb = [](obs_scene_t*, obs_sceneitem_t* sceneItem, void* param) {
		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
		refreshBrowserSource(itemSource);
		return true;
	};
	doRunObsCallbackOnScenes(flagAllScenes, cb, NULL, false, !flagAllScenes);
}


void JrCft::restartMediaSourcesInScenes(bool flagAllScenes) {
	// callback used on each scene item
	auto cb = [](obs_scene_t*, obs_sceneitem_t* sceneItem, void* param) {
		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
		restartMediaSource(itemSource);
		return true;
	};
	doRunObsCallbackOnScenes(flagAllScenes, cb, NULL, false, !flagAllScenes);
}
//---------------------------------------------------------------------------























//---------------------------------------------------------------------------
// helper to launch stuff on start of streaming AND recording
void JrCft::doOnStrRecStartStuff(enum obs_frontend_event event) {
	if (restartMediaOnStart) {
		restartMediaSourcesInScenes(false);
	}
	if (startRecStrCommandline!= "" && !startRecStrCommandline.startsWith("//")) {
		clock_t nowTime = clock();
		clock_t sinceLastRun = nowTime - lastTimeRunCommandline;
		if (sinceLastRun < DefMinimumTimeBetweenStartCommandlineRunSecs * CLOCKS_PER_SEC) {
			return;
		}
		lastTimeRunCommandline = nowTime;
		//
		bool optionStartMinimized = true;
		qtHelpLaunchCommandline(startRecStrCommandline, optionStartMinimized);
	}
}
//---------------------------------------------------------------------------



