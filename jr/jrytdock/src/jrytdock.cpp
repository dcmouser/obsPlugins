#include "jrytdock.hpp"
#include "jrytdock_options.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>

#include <string>


#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>

#include <browser-panel.hpp>





//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define TIMER_INTERVAL 10000
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
static jrYtDock* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = true;

bool obs_module_load() {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	obs_frontend_push_ui_translation(obs_module_get_string);
	//
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	moduleInstance = new jrYtDock(main_window);
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
jrYtDock::jrYtDock(QWidget* parent)
	: jrObsPlugin(),
	QDockWidget(parent),
	timer(this)
{
	initialStartup();

	// build the dock ui
	buildUi();
}


jrYtDock::~jrYtDock()
{
	blog(LOG_WARNING, "deleting.");

	finalShutdown();

	// this can get called by OBS so we null out the global static pointer if so
	if (moduleInstance == this) {
		moduleInstance = NULL;
	}
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void jrYtDock::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	jrYtDock *pluginp = reinterpret_cast<jrYtDock *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}

void jrYtDock::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	jrYtDock *pluginp = reinterpret_cast<jrYtDock *>(data);
	//
	pluginp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void jrYtDock::handleObsFrontendEvent(enum obs_frontend_event event) {
	// let parent handle some cases
	jrObsPlugin::handleObsFrontendEvent(event);
}


void jrYtDock::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
JrPluginOptionsDialog* jrYtDock::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrYtDock::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog)); };
	//
void jrYtDock::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	//
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void jrYtDock::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
}

void jrYtDock::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void jrYtDock::loadStuff(obs_data_t *settings) {
	//
}

void jrYtDock::saveStuff(obs_data_t *settings) {
	//
}
//---------------------------------------------------------------------------




























































//---------------------------------------------------------------------------
void jrYtDock::showEvent(QShowEvent *) {
	timer.start(TIMER_INTERVAL);
}

void jrYtDock::hideEvent(QHideEvent *) {
	timer.stop();
}




void jrYtDock::closeEvent(QCloseEvent *event) {
	if (isVisible()) {
		config_set_string(obs_frontend_get_global_config(), "jrYtDock", "geometry", saveGeometry().toBase64().constData());
		//config_save(obs_frontend_get_global_config());
		//config_save_safe(obs_frontend_get_global_config(), "tmp", nullptr);
	}
	QWidget::closeEvent(event);
}
//---------------------------------------------------------------------------



































































//---------------------------------------------------------------------------
void jrYtDock::buildUi() {
	// what does this do?
	bool deleteOnClose = true;

	// ?
	setObjectName(PLUGIN_NAME);

	setFloating(true);
	hide();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	auto *dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);
	setWidget(dockWidgetContents);



	// bottom buttons
	QPushButton *resetButton = new QPushButton(QTStr("Reset"));
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(resetButton);
	connect(resetButton, &QPushButton::clicked, [this]() { Reset(); });

	mainLayout->addStretch();
	mainLayout->addLayout(buttonLayout);

	resize(640, 480);
	setWindowTitle(QTStr(PLUGIN_NAME));

#ifdef __APPLE__
	setWindowIcon(
		QIcon::fromTheme("obs", QIcon(":/res/images/obs_256x256.png")));
#else
	setWindowIcon(QIcon::fromTheme("obs", QIcon(":/res/images/obs.png")));
#endif

	setWindowModality(Qt::NonModal);

	// ?
	if (deleteOnClose) {
		setAttribute(Qt::WA_DeleteOnClose, true);
	}

	// timers
	QObject::connect(&timer, &QTimer::timeout, this, &jrYtDock::Update);
	timer.setInterval(TIMER_INTERVAL);
	if (isVisible())
		timer.start();

	// initial update
	Update();


	// ATTN: move this
	if (true) {
		const char *geometry = config_get_string(obs_frontend_get_global_config(), "jrYtDock", "geometry");
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

	// add dock
	obs_frontend_add_dock(this);
}




























//---------------------------------------------------------------------------
void jrYtDock::Update()
{
	if (!flagObsIsFullyLoaded) {
		// avoid running this if not loaded
		return;
	}

	struct obs_video_info ovi = {};
	obs_get_video_info(&ovi);
	
	OBSOutputAutoRelease strOutput = obs_frontend_get_streaming_output();
	OBSOutputAutoRelease recOutput = obs_frontend_get_recording_output();
	if (!strOutput && !recOutput)
		return;

	if (firstUpdate) {
		firstUpdate = false;
	}
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
void jrYtDock::Reset()
{
	timer.start();
	Update();
}
//---------------------------------------------------------------------------

