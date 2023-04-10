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
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
static JrCft* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load() {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	obs_frontend_push_ui_translation(obs_module_get_string);
	//
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	moduleInstance = new JrCft(main_window);
	//
	obs_frontend_pop_ui_translation();

	return true;
}

void obs_module_unload() {
	if (moduleInstance != NULL) {
		moduleInstance->onModuleUnload();
	}

	if (moduleInstanceIsRegisteredAndAutoDeletedByObs) {
		blog(LOG_INFO, "plugin managed by and should be auto deleted by OBS.");
		return;
	}
	//blog(LOG_INFO, "plugin unloading");
	//
	if (moduleInstance != NULL) {
		delete moduleInstance;
		moduleInstance = NULL;
	}
	//blog(LOG_INFO, "plugin unloaded");
}


void obs_module_post_load() {
	//blog(LOG_INFO, "plugin in onModulePostLoad");
	if (moduleInstance != NULL) {
		moduleInstance->onModulePostLoad();
	}
}



void do_frontend_save(obs_data_t *save_data, bool saving, void *data) {
	JrCft *pluginp = reinterpret_cast<JrCft *>(data);
	// ATTN: to do
}
//---------------------------------------------------------------------------




































//---------------------------------------------------------------------------
JrCft::JrCft(QWidget* parent)
	: jrObsPlugin(),
	QDockWidget(parent)
{
	// this will trigger LOAD of settings
	initialStartup();

	// build the dock ui
	buildUi();

	postStartup();
}


JrCft::~JrCft()
{
	initialShutdown();

	destructStuff();

	finalShutdown();

	// this can get called by OBS so we null out the global static pointer if so
	if (moduleInstance == this) {
		moduleInstance = NULL;
	}
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrCft::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);

	// hotkeys
	registerHotkey(ObsHotkeyCallback, this, "trigger", hotkeyId_trigger, "Trigger hotkey test");

	// add a save callback?
	obs_frontend_add_save_callback(do_frontend_save, this);
}

void JrCft::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);

	// hotkeys
	unRegisterHotkey(hotkeyId_trigger);

	// save callback
	obs_frontend_remove_save_callback(do_frontend_save, this);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void JrCft::onModulePostLoad() {
	wsSetupWebsocketStuff();
}

void JrCft::onModuleUnload() {
	wsShutdownWebsocketStuff();
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
JrPluginOptionsDialog* JrCft::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}


void JrCft::optionsFinishedChanging() {
}


void JrCft::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) {
	setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog));
}


void JrCft::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	optionDialog->setOptionRestartMediaOnStart(restartMediaOnStart);
	optionDialog->setOptionStartRecStrCommandline(startRecStrCommandline);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void JrCft::loadStuff(obs_data_t *settings) {
	// hotkeys
	loadHotkey(settings, "trigger", hotkeyId_trigger);
	//
	restartMediaOnStart = obs_data_get_bool(settings, "restartMediaOnStart");
	startRecStrCommandline = obs_data_get_string(settings, "startRecStrCommandline");
}

void JrCft::saveStuff(obs_data_t *settings) {
	// hotkeys
	saveHotkey(settings, "trigger", hotkeyId_trigger);
	//
	obs_data_set_bool(settings, "restartMediaOnStart", restartMediaOnStart);
	obs_data_set_string(settings, "startRecStrCommandline", startRecStrCommandline.toUtf8().constData());
}
//---------------------------------------------------------------------------

































//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void JrCft::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	JrCft *pluginp = reinterpret_cast<JrCft *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}

void JrCft::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	JrCft *pluginp = reinterpret_cast<JrCft *>(data);
	//
	pluginp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void JrCft::handleObsFrontendEvent(enum obs_frontend_event event) {
	switch ((int)event) {
		// handle broadcast selected
		case OBS_FRONTEND_EVENT_STREAMING_STARTED:
			doOnStrRecStartStuff(event);
			break;
		case OBS_FRONTEND_EVENT_RECORDING_STARTED:
			doOnStrRecStartStuff(event);
			break;
		case OBS_FRONTEND_EVENT_BROADCAST_STARTING:
			// this is triggered before broadcast actually begins -- to give it time to restart media i have patched OBS to add a delay
			doOnStrRecStartStuff(event);
			break;
		case OBS_FRONTEND_EVENT_BROADCAST_STARTED:
			// do NOT trigger anything here, this event happens AFTER the broadcast begins
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
			break;
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			break;
		case OBS_FRONTEND_EVENT_TRANSITION_STOPPED:
			break;
	}

	// let parent handle some cases
	jrObsPlugin::handleObsFrontendEvent(event);
}


void JrCft::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
	if (id == hotkeyId_trigger) {
		testHotkeyTriggerAction();
	} 
}



// called by parent qdock class
void JrCft::closeEvent(QCloseEvent *event) {
	if (isVisible()) {
		// save location of dock window -- we may need to do this before obs tries to close so we get geometry of last postion
		config_set_string(obs_frontend_get_global_config(), "JrCft", "geometry", saveGeometry().toBase64().constData());
	}
	QWidget::closeEvent(event);
}
//---------------------------------------------------------------------------



































































//---------------------------------------------------------------------------
void JrCft::buildUi() {
	bool deleteOnClose = moduleInstanceIsRegisteredAndAutoDeletedByObs;

	// window basics
	setObjectName(PLUGIN_NAME);
	setWindowTitle(QTStr(PLUGIN_LABEL));
	setFloating(true);
	hide();

	// build UI
	mainLayout = new QVBoxLayout(this);
	//
	auto* dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);
	setWidget(dockWidgetContents);

	// user iterface elements
	int idx=0;
	auto label = new QLabel(obs_module_text("JrCft Dock has no contents to show; see Tools->JrCft Options to configure."));
	mainLayout->addWidget(label, idx);


	// bottom buttons
	bool optionAlignButtonsBottom = true;
	if (optionAlignButtonsBottom) {
		// this would BOTTOM align buttons
		mainLayout->addStretch();
	}

	// buttons here?

	// resize window and style
	resize(260, 160);
	setWindowModality(Qt::NonModal);


	#ifdef __APPLE__
		setWindowIcon(QIcon::fromTheme("obs", QIcon(":/res/images/obs_256x256.png")));
	#else
		setWindowIcon(QIcon::fromTheme("obs", QIcon(":/res/images/obs.png")));
	#endif

	// delete on close feature?
	// ATTN: should this really be true? this may not delet the object just the window?
	if (deleteOnClose) {
		setAttribute(Qt::WA_DeleteOnClose, true);
	}

	// reload last position
	if (true) {
		const char *geometry = config_get_string(obs_frontend_get_global_config(), "JrCft", "geometry");
		if (geometry != NULL) {
			QByteArray byteArray =
				QByteArray::fromBase64(QByteArray(geometry));
			restoreGeometry(byteArray);
		}

		QRect windowGeometry = normalGeometry();
		if (!WindowPositionValid(windowGeometry)) {
			QRect rect =
				QGuiApplication::primaryScreen()->geometry();
			setGeometry(QStyle::alignedRect(Qt::LeftToRight,
							Qt::AlignCenter, size(),
							rect));
		}
	}

	// initial update of display if there is anything we need to do
	Update();

	// add window as obs managed dock
	obs_frontend_add_dock(this);
}
//---------------------------------------------------------------------------



























//---------------------------------------------------------------------------
void JrCft::Update() {
	// called after UI built, but this is not called automatically, we would have to trigger it manually beyond UI construction
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
void JrCft::destructStuff() {
	wsShutdownWebsocketStuff();
}
//---------------------------------------------------------------------------











































//---------------------------------------------------------------------------
void JrCft::wsSetupWebsocketStuff() {
	//mydebug("In JrYouTubeChat::wsSetupWebsocketStuff.");
	vendor = obs_websocket_register_vendor("JrCft");
	if (!vendor) {
		warn("Failed to register websockets vendor.");
		return;
	}

	// register request clients can make of us: JrYtMessageSelect
	auto event_request_cb_JrCommand = [](obs_data_t *request_data, obs_data_t *response_data,
					void *thisptr) {
		//mydebug("IN cb JrYtCommand.");
		JrCft* ytp = static_cast<JrCft*>(thisptr);
		// handle callback
		ytp->requestWsHandleCommandByClient(request_data, response_data);
	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "JrCftCommand", event_request_cb_JrCommand, this)) {
		warn("Failed to register obs - websocket request event_request_cb_JrCommand");
	}
}


void JrCft::wsShutdownWebsocketStuff() {
	if (vendor) {
		// concerned about crashing
		//obs_websocket_vendor_unregister_request(vendor, "JrCftCommand");
		vendor = NULL;
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrCft::postStartup() {
}

void JrCft::initialShutdown() {
}
//---------------------------------------------------------------------------



















