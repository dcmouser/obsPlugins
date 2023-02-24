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

#include "plugininfo.hpp"

#include "../../jrcommon/src/jrobsplugin.hpp"



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

class JrYouTubeId : public QDockWidget, public jrObsPlugin {
	// produces link error:
	//Q_OBJECT
	QVBoxLayout* mainLayout;

protected:
	// from scene notes plugin
	QLineEdit *editYouTubeId;
	QString youTubeIdQstr;
	QString chatUtilityCommandLine;
	bool optionStartMinimized = false;
	bool dirtyChanges = false;
	qint64 chatExePid = 0;
public:
	void destructStuff();
protected:
	QTimer timer;
public:
	JrYouTubeId(QWidget *parent = nullptr);
	~JrYouTubeId();
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
	void setDerivedSettingsOnOptionsDialog(OptionsDialog *optionDialog);
protected:
	virtual void loadStuff(obs_data_t *settings);
	virtual void saveStuff(obs_data_t *settings);

protected:
	void buildUi();
protected:
	void Update();
protected:
	virtual void closeEvent(QCloseEvent *event) override;
protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };
protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;

public slots:
	void Reset();
	void goDistribute();
	void goSetBrowserChatIds();
	void goLaunchChatUtility();
	void goOpenYtWebPage();
public:
	void grabVideoIdFromObsSelectedBroadcast();
	void sendYoutubeIdToBrowserChatSources(const QString videoid);
	void launchChatMonitorUtility(QString videoid);
	void receiveYoutubeIdSelectedSignal(QString videoid);
public:
	void setOptionChatUtilityCommandline(QString inChatUtilityCommandLine);
	void setOptionStartMinimized(bool val);
};
//---------------------------------------------------------------------------

