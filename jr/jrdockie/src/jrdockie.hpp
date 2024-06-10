#pragma once

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>
#include <QDockWidget>
#include <QPointer>
#include <QWidget>
#include <QLabel>
#include <QList>

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
#define OBS_TOPLEVEL_MENU_LABEL "Docks"
#define JRDOCKIE_MENU_LABEL "Dock Sets"
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------

class JrDockie : public QObject, jrObsPlugin {
	Q_OBJECT
protected:
	QString opt_topLevelDockLabel;
protected:
	QMainWindow* qmainwp;
public:
	JrDockie(QMainWindow* in_qmainwp);
	~JrDockie();
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
public:
	virtual void optionsFinishedChanging();
protected:
	virtual void loadStuff(obs_data_t *settings);
	virtual void saveStuff(obs_data_t *settings);

protected:
	void buildUi();
protected:
	void Update();
protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };

public:
	void setOptionTopLevelDockLabel(QString qstr) { opt_topLevelDockLabel = qstr; }

public slots:
	void Reset();



// ATTN: new dock set support funcs
public slots:
	void on_actionExportDockset_triggered();
	void on_actionImportDockset_triggered();
	void on_actionBrowseDocksets_triggered();
	void on_actionRefreshDocksets_triggered();
	void on_actionShowOptions_triggered();
	void OnClickRecentDockset();
public:
	int GetConfigPathDockset(char *path, int maxlen);
	bool ExportDockstateToFileUserChooses();
	bool ImportDockstateFromFileUserChooses();
	bool ExportDockstateToFile(QString filepath);
	bool ImportDockstateFromFile(QString filepath);
	bool ImportDockstateFromCharp(const char *dockStateStr);
public:
	void RefreshDocksetRecentMenu();
	void EnumDocksetFiles(std::function<bool(std::string name, const char *, size_t index)> &&cb);
public:
	QMenu* getDockMenuWidgetp();
	void hideDockMenuWidgetp();
};
//---------------------------------------------------------------------------

