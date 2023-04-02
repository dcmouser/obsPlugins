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
	} else {
		mydebug("Unknown requestWsHandleCommandByClient: %s.", comStr);
		obs_data_set_string(response_data, "error", "Unknown command issued to vendor (JrCft).");
	}
}


void JrCft::testHotkeyTriggerAction() {
	setSourceVisiblityByName(true, "Overlay - Front Inset", JrForceSourceStateToggle);
}
//---------------------------------------------------------------------------

