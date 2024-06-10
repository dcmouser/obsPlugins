
#include "pluginInfo.hpp"
//
#include "jrdockie.hpp"
#include "jrdockie_options.hpp"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>

#include <string>


#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>
#include <QtCore/qfile.h>
#include <QString>
#include <QMenuBar>




//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
static JrDockie* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load() {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	obs_frontend_push_ui_translation(obs_module_get_string);
	//
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	moduleInstance = new JrDockie(main_window);
	//
	// TEST
	moduleInstance->RefreshDocksetRecentMenu();
	//
	obs_frontend_pop_ui_translation();

	return true;
}

void obs_module_unload() {
	if (moduleInstanceIsRegisteredAndAutoDeletedByObs) {
		blog(LOG_INFO, "plugin managed by and should be auto deleted by OBS.");
		return;
	}
	blog(LOG_INFO, "plugin unloading");
	//
	if (moduleInstance != NULL) {
		delete moduleInstance;
		moduleInstance = NULL;
	}
	blog(LOG_INFO, "plugin unloaded");
}
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
JrDockie::JrDockie(QMainWindow* in_qmainwp)
	: jrObsPlugin(),
	QObject(in_qmainwp)
{
	// record main window
	qmainwp = (QMainWindow*) in_qmainwp;

	// this will trigger LOAD of settings
	initialStartup();

	// build the dock ui
	buildUi();
}


JrDockie::~JrDockie()
{
	finalShutdown();

	// this can get called by OBS so we null out the global static pointer if so
	if (moduleInstance == this) {
		moduleInstance = NULL;
	}
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void JrDockie::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	JrDockie *pluginp = reinterpret_cast<JrDockie *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}

void JrDockie::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	JrDockie *pluginp = reinterpret_cast<JrDockie *>(data);
	//
	pluginp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void JrDockie::handleObsFrontendEvent(enum obs_frontend_event event) {
	// let parent handle some cases
	jrObsPlugin::handleObsFrontendEvent(event);
}


void JrDockie::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
JrPluginOptionsDialog* JrDockie::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrDockie::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog)); };
	//
void JrDockie::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	//
	optionDialog->setOptionTopMenuLabel(opt_topLevelDockLabel);
}

void JrDockie::optionsFinishedChanging() {
	// parent?
	//jrObsPlugin::optionsFinishedChanging();
	RefreshDocksetRecentMenu();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrDockie::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
}

void JrDockie::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void JrDockie::loadStuff(obs_data_t *settings) {
	//
	//
	const char* charp = obs_data_get_string(settings, "topLevelMenuLabel");
	if (charp != NULL) {
		opt_topLevelDockLabel = QString(charp);
	} else {
		// default
		opt_topLevelDockLabel = QString(OBS_TOPLEVEL_MENU_LABEL);;
	}
}

void JrDockie::saveStuff(obs_data_t *settings) {
	//
	obs_data_set_string(settings, "topLevelMenuLabel", opt_topLevelDockLabel.toStdString().c_str());
}
//---------------------------------------------------------------------------





























































































































//---------------------------------------------------------------------------
void JrDockie::buildUi() {
	// what does this do?
	bool deleteOnClose = moduleInstanceIsRegisteredAndAutoDeletedByObs;

	// initial update
	Update();
}
//---------------------------------------------------------------------------



























//---------------------------------------------------------------------------
void JrDockie::Update()
{
	if (!flagObsIsFullyLoaded) {
		// avoid running this if not loaded
		return;
	}


	if (firstUpdate) {
		firstUpdate = false;
	}
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
void JrDockie::Reset()
{
	Update();
}
//---------------------------------------------------------------------------






























































// ATTN: jr 9/25/22 dock saving and loading
#define DefDocksetFileExtension ".dockset"

void JrDockie::on_actionExportDockset_triggered()
{
	ExportDockstateToFileUserChooses();
}
void JrDockie::on_actionImportDockset_triggered()
{
	ImportDockstateFromFileUserChooses();
}
void JrDockie::on_actionBrowseDocksets_triggered()
{
	char path[512];
	int ret = GetConfigPathDockset(path, 512);
	if (ret <= 0) {
		return;
	}
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
void JrDockie::on_actionRefreshDocksets_triggered()
{
	RefreshDocksetRecentMenu();
}

void JrDockie::on_actionShowOptions_triggered()
{
	triggerOptionsShow();
}

int JrDockie::GetConfigPathDockset(char *path, int maxlen) {
	int retv = createModuleConfigSubdir(path, maxlen, "docksets");
	//mydebug("returning as dockset dir: '%s'.", path);
	return 1;
}




bool JrDockie::ExportDockstateToFileUserChooses()
{
	char path[512];
	int ret = GetConfigPathDockset(path, 512);
	if (ret <= 0) {
		return false;
	}
	QString currentFile = QT_UTF8("mydocks");
	QString filePath = SaveFile(
		qmainwp, QTStr("Basic.MainMenu.DockSet.Export"),
		QString(path) + "/" + currentFile,
		"Dockset Files (*" + QString(DefDocksetFileExtension) + ")");

	if (!filePath.isEmpty() && !filePath.isNull()) {
		if (QFile::exists(filePath))
			QFile::remove(filePath);
		return ExportDockstateToFile(filePath);
	}
	return false;
}

bool JrDockie::ImportDockstateFromFileUserChooses()
{
	char path[512];
	int ret = GetConfigPathDockset(path, 512);
	if (ret <= 0) {
		return false;
	}
	QString currentFile = QT_UTF8("mydocks");
	QString filePath = OpenFile(
		qmainwp, QTStr("Basic.MainMenu.DockSet.Import"),
		QString(path) + "/" + currentFile,
		"Dockset Files (*" + QString(DefDocksetFileExtension) + ")");

	if (!filePath.isEmpty() && !filePath.isNull()) {
		if (!QFile::exists(filePath))
			return false;
		return ImportDockstateFromFile(filePath);
	}
	return false;
}

bool JrDockie::ExportDockstateToFile(QString filepath)
{
	const char *dockStateCharp = qmainwp->saveState().toBase64().constData();
	if (!dockStateCharp) {
		return false;
	}
	bool success = os_quick_write_utf8_file(filepath.toUtf8(),
						dockStateCharp,
						strlen(dockStateCharp), false);
	if (success) {
		RefreshDocksetRecentMenu();
	}
	return true;
}

bool JrDockie::ImportDockstateFromFile(QString filepath)
{
	char *dockStateCharp = os_quick_read_utf8_file(filepath.toUtf8());
	if (!dockStateCharp) {
		return false;
	}
	bool success = ImportDockstateFromCharp(dockStateCharp);
	bfree(dockStateCharp);
	if (success) {
		RefreshDocksetRecentMenu();
	}
	return success;
}

bool JrDockie::ImportDockstateFromCharp(const char *dockStateStr)
{
	if (dockStateStr == NULL) {
		return false;
	}
	QByteArray dockState = QByteArray::fromBase64(QByteArray(dockStateStr));
	return qmainwp->restoreState(dockState);
}

// recent dockset menu code based on https://het.as.utexas.edu/HET/Software/html/mainwindows-recentfiles-mainwindow-cpp.html and https://www.walletfox.com/course/qtopenrecentfiles.php
void JrDockie::OnClickRecentDockset()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		QVariant v = action->property("file_name");
		ImportDockstateFromFile(v.toString());
	}
}

void JrDockie::RefreshDocksetRecentMenu()
{
	QList<QAction*> menuActions;

	QMenu* menup = getDockMenuWidgetp();
	if (menup == NULL) {
		//mydebug("Null from getDockMenuWidgetp.");
		return;
	}
	menuActions = menup->actions();
	//mydebug("Got menu actions.");

	// add all docksets found in directory
	char path[512];
	int ret = GetConfigPathDockset(path, 512);
	if (ret < 0) {
		return;
	}


	// clear existing actions
	for (int i = 0; i < menuActions.count(); i++) {
		if (true) {
			// delete all in menu
			delete menuActions[i];
		}
		else {
			QVariant v = menuActions[i]->property("file_name");
			if (!v.isValid()) {
				continue;
			}
			if (v.typeName() != nullptr)
				delete menuActions[i];
		}
	}




	// add special actions
	QAction* action;
	// now add separator
	//menup->addSeparator();
	//
	action = new QAction(QT_UTF8("&Import dockset.."), this);
	connect(action, &QAction::triggered, this, &JrDockie::on_actionImportDockset_triggered);
	menup->addAction(action);
	//
	action = new QAction(QT_UTF8("&Export dockset.."), this);
	connect(action, &QAction::triggered, this, &JrDockie::on_actionExportDockset_triggered);
	menup->addAction(action);
	//
	menup->addSeparator();
	//
	action = new QAction(QT_UTF8("&Browse dockset file directory"), this);
	connect(action, &QAction::triggered, this, &JrDockie::on_actionBrowseDocksets_triggered);
	menup->addAction(action);
	//
	action = new QAction(QT_UTF8("&Refresh this menu"), this);
	connect(action, &QAction::triggered, this, &JrDockie::on_actionRefreshDocksets_triggered);
	menup->addAction(action);
	//
	action = new QAction(QT_UTF8("&JrDockie options.."), this);
	connect(action, &QAction::triggered, this, &JrDockie::on_actionShowOptions_triggered);
	menup->addAction(action);

	// separator
	menup->addSeparator();

	// add actions for enumerate files found
	auto addRecentDocksetFile = [&](std::string name, const char *path, size_t index) {
		if (index < 9) {
			name = "&" + std::to_string(index+1) + ". " + name;
		}
		QAction *action = new QAction(QT_UTF8(name.c_str()), this);
		action->setProperty("file_name", QT_UTF8(path));
		connect(action, &QAction::triggered, this,
			&JrDockie::OnClickRecentDockset);
		menup->addAction(action);
		return true;
	};
	EnumDocksetFiles(addRecentDocksetFile);


	// ensure visible
	menup->menuAction()->setVisible(true);
}




void JrDockie::EnumDocksetFiles(
	std::function<bool(std::string name, const char *, size_t index)> &&cb)
{
	// based on code in EnumSceneCollections()
	char path[512];
	int ret = GetConfigPathDockset(path, 512);
	if (ret < 0) {
		return;
	}
	strcat(path, "/*");
	strcat(path, DefDocksetFileExtension);

	os_glob_t *glob;
	if (os_glob(path, 0, &glob) != 0) {
		blog(LOG_WARNING, "Failed to glob dockset file path");
		return;
	}

	for (size_t i = 0; i < glob->gl_pathc; i++) {
		const char *filePath = glob->gl_pathv[i].path;

		if (glob->gl_pathv[i].directory)
			continue;

		std::string name = strrchr(filePath, '/') + 1;
		// remove the extension
		size_t dotpos = name.find_last_of('.');
		if (dotpos != std::string::npos) {
			name.resize(dotpos);
		}

		// callback
		if (!cb(name, filePath, i))
			break;
	}
	os_globfree(glob);
}






QMenu* JrDockie::getDockMenuWidgetp() {
	// Ideally:
	// step 1 find the main top-level dock menu
	// if not found, its an error
	// step 2: find the DockIO submenu -- if not found, create it
	// step 3: return the DockIO submenu

	// options
	bool flagPreferUnderDockMenu = true;
	bool flagInsertAtTop = true;
	bool flagCreateTopLevelIfNoTopDockMenuFound = true;

	//
	QMenu* docksSubmenuChildMenu = NULL;
	QMenu* docksSubmenuMenu = NULL;

	// step 1
	QAction* docksSubmenuAction = NULL;
	QMenuBar* mainMenubarp = qmainwp->menuBar();
	if (mainMenubarp == NULL) {
		mydebug("qmainwp->menuBar yielded null.");
		return NULL;
	}
	QString topMenuWithAmbersand = "&" + opt_topLevelDockLabel;
	foreach(QAction * action, mainMenubarp->actions()) {
		if (action->menu()) {
			// found a submenu, is it "Docks"?
			//mydebug("Found toplevel menu: '%s'.", action->text().toStdString());
			if (flagPreferUnderDockMenu && opt_topLevelDockLabel!="" && ((action->text() == opt_topLevelDockLabel) || (action->text() == topMenuWithAmbersand))) {
				// found it
				docksSubmenuAction = action;
				break;
			}
			if (action->text() == JRDOCKIE_MENU_LABEL) {
				// found Our OWN top level
				docksSubmenuAction = action;
				docksSubmenuChildMenu = action->menu();
				break;
			}
		}
	}
	if (docksSubmenuAction == NULL) {
		if (flagCreateTopLevelIfNoTopDockMenuFound || !flagPreferUnderDockMenu) {
			docksSubmenuChildMenu = mainMenubarp->addMenu(JRDOCKIE_MENU_LABEL);
		}
		else {
			mydebug("Could not find top level Docks submenu.");
			return NULL;
		}
	}

	//mydebug("Found top level Docks submenu.");

	// setp 2, find child (unless we are putting ourselves at TOP level)
	if (docksSubmenuChildMenu == NULL && docksSubmenuAction!=NULL) {
		QAction* docksSubmenuChildAction = NULL;
		docksSubmenuMenu = docksSubmenuAction->menu();
		foreach(QAction * action, docksSubmenuMenu->actions()) {
			if (action->menu()) {
				// found a submenu, see if its ud
				if (action->text() == JRDOCKIE_MENU_LABEL) {
					// found it
					docksSubmenuChildAction = action;
					docksSubmenuChildMenu = action->menu();
					break;
				}
			}
		}
	}

	if (docksSubmenuChildMenu == NULL) {
		// does not exist, we need to create and add it
		// create it
		//docksSubmenuChildAction = new QAction(JRDOCKIE_MENU_LABEL, docksSubmenuAction->menu());
		// add it
		if (flagInsertAtTop) {
			auto itemBefore = docksSubmenuMenu->actions()[0];
			docksSubmenuAction->menu()->insertSeparator(itemBefore);
			docksSubmenuChildMenu = new QMenu(JRDOCKIE_MENU_LABEL, docksSubmenuMenu);
			docksSubmenuAction->menu()->insertMenu(itemBefore, docksSubmenuChildMenu);
			docksSubmenuAction->menu()->insertSeparator(itemBefore);
		}
		else {
			docksSubmenuAction->menu()->addSeparator();
			docksSubmenuChildMenu = docksSubmenuAction->menu()->addMenu(JRDOCKIE_MENU_LABEL);
			docksSubmenuAction->menu()->addSeparator();
		}
		//mydebug("Created docksSubmenuChildAction.");
	}
	else {
		//mydebug("Found docksSubmenuChildAction.");
	}

	// return it
	return docksSubmenuChildMenu;
}


void JrDockie::hideDockMenuWidgetp() {
	QMenu* menup = getDockMenuWidgetp();
	if (menup != NULL) {
		menup->menuAction()->setVisible(false);
	}
}
//---------------------------------------------------------------------------
