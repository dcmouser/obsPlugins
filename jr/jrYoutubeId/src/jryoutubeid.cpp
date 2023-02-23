#include "jryoutubeid.hpp"
#include "jryoutubeid_options.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>
#include <obs.h>
#include <string>

#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>

#include <QProcess>
#include <QDesktopServices>

#include <windows.h>
#include <shellapi.h>


//#include <../deps/json11/json11.hpp>
//#include <../auth-youtube.hpp>
//#include <../youtube-api-wrappers.hpp>


//---------------------------------------------------------------------------
#define DefLaunchChatUtility false
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define TIMER_INTERVAL 10000
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
extern "C" {
	// prototype for custom obs function we need to export from obs
	extern const char* obs_get_broadcastid_str();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
static JrYouTubeId* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load() {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	obs_frontend_push_ui_translation(obs_module_get_string);
	//
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	moduleInstance = new JrYouTubeId(main_window);
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
JrYouTubeId::JrYouTubeId(QWidget* parent)
	: jrObsPlugin(),
	QDockWidget(parent),
	timer(this)
{
	// this will trigger LOAD of settings
	initialStartup();

	// build the dock ui
	buildUi();
}


JrYouTubeId::~JrYouTubeId()
{
	destructStuff();

	finalShutdown();

	// this can get called by OBS so we null out the global static pointer if so
	if (moduleInstance == this) {
		moduleInstance = NULL;
	}
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void JrYouTubeId::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	JrYouTubeId *pluginp = reinterpret_cast<JrYouTubeId *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}

void JrYouTubeId::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	JrYouTubeId *pluginp = reinterpret_cast<JrYouTubeId *>(data);
	//
	pluginp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void JrYouTubeId::handleObsFrontendEvent(enum obs_frontend_event event) {
	switch ((int)event) {
		// handle broadcast selected
		case OBS_FRONTEND_EVENT_BROADCAST_SELECTED:
			grabVideoIdFromObsSelectedBroadcast();
			break;
	}

	// let parent handle some cases
	jrObsPlugin::handleObsFrontendEvent(event);
}


void JrYouTubeId::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
JrPluginOptionsDialog* JrYouTubeId::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrYouTubeId::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) {
	setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog));
};
//
void JrYouTubeId::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	optionDialog->setOptionChatUtilityCommandline(chatUtilityCommandLine);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeId::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
}

void JrYouTubeId::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void JrYouTubeId::loadStuff(obs_data_t *settings) {
	const char *ytsBuf = obs_data_get_string(settings, "youtubeid");
	youTubeIdQstr = QString(ytsBuf);
	ytsBuf = obs_data_get_string(settings, "chatComline");
	chatUtilityCommandLine = QString(ytsBuf);
}

void JrYouTubeId::saveStuff(obs_data_t *settings) {
	auto yts = editYouTubeId->text();
	obs_data_set_string(settings, "youtubeid", yts.toUtf8().constData());
	obs_data_set_string(settings, "chatComline", chatUtilityCommandLine.toUtf8().constData());
	dirtyChanges = false;
}
//---------------------------------------------------------------------------




























































//---------------------------------------------------------------------------
void JrYouTubeId::showEvent(QShowEvent *) {
	timer.start(TIMER_INTERVAL);
}

void JrYouTubeId::hideEvent(QHideEvent *) {
	timer.stop();
}




void JrYouTubeId::closeEvent(QCloseEvent *event) {
	if (isVisible()) {
		config_set_string(obs_frontend_get_global_config(), "JrYouTubeId", "geometry", saveGeometry().toBase64().constData());
	}
	QWidget::closeEvent(event);
}
//---------------------------------------------------------------------------



































































//---------------------------------------------------------------------------
void JrYouTubeId::buildUi() {
	// what does this do?
	bool deleteOnClose = moduleInstanceIsRegisteredAndAutoDeletedByObs;

	setObjectName(PLUGIN_NAME);
	setFloating(true);
	hide();

	mainLayout = new QVBoxLayout(this);

	auto *dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);
	setWidget(dockWidgetContents);



	// main text inputs
	//
	// label
	QLabel* youtubeIdLabel = new QLabel(this);
	youtubeIdLabel->setText("YouTube Video Id:");
	mainLayout->addWidget(youtubeIdLabel);
	// the text edit
	editYouTubeId = new QLineEdit(this);
	//
	mainLayout->addWidget(editYouTubeId);
	// set deferred initial values
	editYouTubeId->setText(youTubeIdQstr);


	// bottom buttons
	if (true) {
		//QHBoxLayout *buttonLayout = new QHBoxLayout;
		QVBoxLayout *buttonLayout = new QVBoxLayout;
		buttonLayout->addStretch();

		// buttons
		QHBoxLayout *buttonLayoutInner = new QHBoxLayout;
		QPushButton *chatUpdateButton = new QPushButton(QTStr("Set chat urls"));
		buttonLayoutInner->addWidget(chatUpdateButton);
		connect(chatUpdateButton, &QPushButton::clicked, [this]() { goSetBrowserChatIds(); });
		//
		QPushButton *visitUrlButton = new QPushButton(QTStr("Open browser"));
		buttonLayoutInner->addWidget(visitUrlButton);
		connect(visitUrlButton, &QPushButton::clicked, [this]() { goOpenYtWebPage(); });
		//
		buttonLayout->addLayout(buttonLayoutInner);

		//
		QPushButton *chatUtilityLaunchButton = new QPushButton(QTStr("Launch chat utility"));
		buttonLayout->addWidget(chatUtilityLaunchButton);
		connect(chatUtilityLaunchButton, &QPushButton::clicked, [this]() { goLaunchChatUtility(); });

		mainLayout->addStretch();
		mainLayout->addLayout(buttonLayout);
		}

	resize(640, 480);
	setWindowTitle(QTStr(PLUGIN_LABEL));

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
	QObject::connect(&timer, &QTimer::timeout, this, &JrYouTubeId::Update);
	timer.setInterval(TIMER_INTERVAL);
	if (isVisible())
		timer.start();

	// initial update
	Update();

	// ATTN: move this
	if (true) {
		const char *geometry = config_get_string(obs_frontend_get_global_config(), "JrYouTubeId", "geometry");
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

	// add a save callback?
	obs_frontend_add_save_callback(do_frontend_save, this);
}
//---------------------------------------------------------------------------



























//---------------------------------------------------------------------------
void JrYouTubeId::Update()
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
void JrYouTubeId::Reset()
{
	timer.start();
	Update();
}
//---------------------------------------------------------------------------





























//---------------------------------------------------------------------------
void do_frontend_save(obs_data_t *save_data, bool saving, void *data) {
	JrYouTubeId *pluginp = reinterpret_cast<JrYouTubeId *>(data);
}








void JrYouTubeId::destructStuff() {
	obs_frontend_remove_save_callback(do_frontend_save, this);
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
void JrYouTubeId::setOptionChatUtilityCommandline(QString inChatUtilityCommandLine) {
	chatUtilityCommandLine = inChatUtilityCommandLine;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrYouTubeId::goDistribute() {
	// youtube id
	auto yts = editYouTubeId->text();
	// two things happen here, FIRST we send the video id to any chat browser sources
	sendYoutubeIdToBrowserChatSources(yts);
	// second, we launch our blinkchat commandline tool (or whatever is configured) with video id in the commandline
	if (DefLaunchChatUtility) {
		launchChatMonitorUtility(yts);
	}
}



void JrYouTubeId::goSetBrowserChatIds() {
	// youtube id
	auto yts = editYouTubeId->text();
	// two things happen here, FIRST we send the video id to any chat browser sources
	sendYoutubeIdToBrowserChatSources(yts);
}


void JrYouTubeId::goLaunchChatUtility() {
	// youtube id
	auto yts = editYouTubeId->text();
	// two things happen here, FIRST we send the video id to any chat browser sources
	launchChatMonitorUtility(yts);
}

void JrYouTubeId::goOpenYtWebPage() {
	// youtube id
	auto yts = editYouTubeId->text();
	auto url = "https://www.youtube.com/watch?v=" + yts;
	QDesktopServices::openUrl(url);
}

void JrYouTubeId::grabVideoIdFromObsSelectedBroadcast() {
	mydebug("ATTN: in grabVideoIdFromObsSelectedBroadcast.");
	QString broadcastIdQstr = "";

	// ask obs for broadcast id
	if (true) {
		const char* broadcastIdStrbuf = obs_get_broadcastid_str();
		if (broadcastIdStrbuf) {
			broadcastIdQstr = QString(broadcastIdStrbuf);
		}
	}

	if (false) {
		// try to get form service?
		obs_service_t* servicep = obs_frontend_get_streaming_service();
		if (servicep) {
			obs_data_t* serviceSettings = obs_service_get_settings(servicep);
			if (serviceSettings) {
				mydebug("settings: %s", obs_data_get_json(serviceSettings));
				const char* streamid = obs_data_get_string(serviceSettings, "stream_id");
				if (streamid) {
					broadcastIdQstr = QString(streamid);
				}
				obs_data_release(serviceSettings);
			}
		}
	}

	receiveYoutubeIdSelectedSignal(broadcastIdQstr);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// TODO: catch a custom signal that a broadcast video id has been selected, then force the youtube channel id to it,
void JrYouTubeId::receiveYoutubeIdSelectedSignal(QString videoid) {
	youTubeIdQstr = videoid;
	editYouTubeId->setText(youTubeIdQstr);
	// send it to browser chats
	sendYoutubeIdToBrowserChatSources(videoid);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeId::launchChatMonitorUtility(QString videoid) {
	if (videoid == "") {
		return;
	}
	// launch the chat executable with our videoid
	// build it from comline, but replace %VIDEOID% with videoid and %OUTPATH% with base path to record path and basename
	QString comlineq = chatUtilityCommandLine;
	//
	std::string filePrefix = ""; // videoid.toStdString() + " ";
	std::string fileSuffix = "";
	std::string outpath = calcTimestampFilePath(filePrefix, fileSuffix);
	QString outpathq = QString::fromStdString(outpath);
	//
	comlineq = comlineq.replace("%VIDEOID%", videoid);
	comlineq = comlineq.replace("%OUTPATH%", outpathq);
	//mydebug("Launching %s.", comlineq.toStdString().c_str());

	QStringList argumentsList =  QProcess::splitCommand(comlineq);
	if (argumentsList.count() < 1) {
		return;
	}

	QProcess process;
	process.setCreateProcessArgumentsModifier(
                [](QProcess::CreateProcessArguments *args) {
		args->flags |= CREATE_NEW_CONSOLE;
		args->startupInfo->dwFlags &=~ STARTF_USESTDHANDLES;
	});
	QString program = argumentsList.takeFirst();

	process.setProgram(program);
	process.setArguments(argumentsList);
	process.startDetached(&chatExePid);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void JrYouTubeId::sendYoutubeIdToBrowserChatSources(const QString videoid) {
	// ok what we want to do now is iterate through all scenes, and for each scene
	// iterate through all sources, and look for a BROWSER source with url pattern of ".*live_chat?v=.*
	// and for those we find, set the v= part to our video id
	if (videoid == "") {
		return;
	}

	auto cb = [](obs_scene_t *, obs_sceneitem_t *sceneItem, void *param) {
		QString* videoidstrp = (QString*)param;
		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
		//auto sourceName = obs_source_get_name(itemSource);
		//auto sourceType = obs_source_get_type(itemSource);
		auto source_id = obs_source_get_unversioned_id(itemSource);
		if (strcmp(source_id,"browser_source")==0) {
			// it's a browser source
			// get current url
                	auto sourceSettings = obs_source_get_settings(itemSource);
			auto url = obs_data_get_string(sourceSettings, "url");
			QString urlq = QString(url);
			// regex check
			//QRegularExpression re("(.+)(live_chat\?v=[^&\?]*)(.*)");
			QRegularExpression re("(.+)(live_chat\\?v=[^&\\?]*)(.*)");
			QRegularExpressionMatch match = re.match(urlq);
			if (!match.isValid()) {
				//mydebug("Match says invalid.");
			}
			if (match.hasMatch()) {
				// built replacement url
				auto pre = match.captured(1);
				auto post = match.captured(3);
				QString newUrlq = pre + QString("live_chat?v=") + *videoidstrp + post;
				if (newUrlq != urlq) {
					// save it
					obs_data_set_string(sourceSettings, "url", newUrlq.toStdString().c_str());
					//mydebug("Changed url from %s to %s.", url, newUrlq.toStdString().c_str());
					// force refresh kludge?
					if (false) {
						auto fps = obs_data_get_int(sourceSettings, "fps");
						if (fps % 2 == 0) {
							obs_data_set_int(sourceSettings, "fps", fps + 1);
						}
						else {
							obs_data_set_int(sourceSettings, "fps", fps - 1);
						}
					}
					obs_source_update(itemSource, sourceSettings);
				}
			}
			// release settings
			obs_data_release(sourceSettings);
		}
		return true;
	};


	// iterate scenes
	obs_frontend_source_list sceneList = {};
	obs_frontend_get_scenes(&sceneList);
	for (size_t i = 0; i < sceneList.sources.num; i++) {
		obs_source_t *sceneSource = sceneList.sources.array[i];
		obs_scene_t* scene = obs_scene_from_source(sceneSource);
		if (scene) {
			// enumerate items in scene
			obs_scene_enum_items(scene, cb, (void*) &videoid);
		}
	}

	// free scene list
	obs_frontend_source_list_free(&sceneList);
}
//---------------------------------------------------------------------------











