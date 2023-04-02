#pragma once


#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>

#include <QDockWidget>
#include <QPointer>
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QList>
#include <QLineEdit>
#include <QListWidget>
#include <QProcess>
#include <QDesktopServices>
#include <QStyledItemDelegate>

#include "plugininfo.hpp"

#include "../../jrcommon/src/jrobsplugin.hpp"
#include "../../plugins/obs-websocket/lib/obs-websocket-api.h"





//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// forward declarations
class QGridLayout;
class QCloseEvent;
//
class OptionsDialog;
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void do_frontend_save(obs_data_t* save_data, bool saving, void* data);

class JrCft : public QDockWidget, public jrObsPlugin {

protected:
	QVBoxLayout* mainLayout;
protected:
	obs_websocket_vendor vendor = NULL;
protected:
	size_t hotkeyId_trigger = -1;
public:
	void destructStuff();
public:
	JrCft(QWidget* parent = nullptr);
	~JrCft();
public:
	virtual void postStartup();
	virtual void initialShutdown();
public:
	static void ObsFrontendEvent(enum obs_frontend_event event, void* ptr);
	static void ObsHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed);
protected:
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
	virtual void handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key);
protected:
	virtual JrPluginOptionsDialog* createNewOptionsDialog();
	virtual void setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog);
protected:
	virtual void registerCallbacksAndHotkeys();
	virtual void unregisterCallbacksAndHotkeys();
public:
	void setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog);
protected:
	virtual void loadStuff(obs_data_t* settings);
	virtual void saveStuff(obs_data_t* settings);
protected:
	void buildUi();
protected:
	void Update();
protected:
	virtual void closeEvent(QCloseEvent* event) override;
protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };
protected:
	void wsSetupWebsocketStuff();
	void wsShutdownWebsocketStuff();
	void requestWsSelectedMessageInfoEvent(obs_data_t *response_data, QListWidgetItem* itemp, int index, bool forInternalUse, bool messageChanged);
	void requestWsAllMessagesInListEvent(obs_data_t *response_data);
	void requestWsHandleMessageSelectedByClient(obs_data_t *request_data, obs_data_t* response_data);
	void requestWsHandleCommandByClient(obs_data_t *request_data, obs_data_t* response_data);
public:
	virtual void onModulePostLoad();
	virtual void onModuleUnload();
public:
	virtual void optionsFinishedChanging();
public:
	void testHotkeyTriggerAction();
};
//---------------------------------------------------------------------------




