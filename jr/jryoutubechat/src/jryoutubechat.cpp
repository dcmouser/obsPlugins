#include "pluginInfo.hpp"
//
#include "jryoutubechat.hpp"
#include "jryoutubechat_options.hpp"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"
//
#include "myQtItemDelegate.hpp"


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
extern "C" {
	// prototype for custom obs function we need to export from obs
	extern const char* obs_get_broadcastid_str();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
static JrYouTubeChat* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load() {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	obs_frontend_push_ui_translation(obs_module_get_string);
	//
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	moduleInstance = new JrYouTubeChat(main_window);
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


// im not sure why this is not being called...
void obs_module_post_load() {
	//blog(LOG_INFO, "plugin in onModulePostLoad");
	if (moduleInstance != NULL) {
		moduleInstance->onModulePostLoad();
	}
}
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
JrYouTubeChat::JrYouTubeChat(QWidget* parent)
	: jrObsPlugin(),
	QDockWidget(parent),
	autoTimer(this)
{
	// this will trigger LOAD of settings
	initialStartup();

	// build the dock ui
	buildUi();

	postStartup();
}


JrYouTubeChat::~JrYouTubeChat()
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
//
void JrYouTubeChat::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	optionDialog->setOptionChatUtilityCommandline(chatUtilityCommandLine);
	optionDialog->setOptionStartEmbedded(optionStartEmbedded);
	optionDialog->setOptionShowEmoticons(optionShowEmoticons);
	optionDialog->setOptionSetStyle(optionSetStyle);
	//
	optionDialog->setOptionFontSize(optionFontSize);
	optionDialog->setOptionDefaultAvatarUrl(optionDefaultAvatarUrl);
	optionDialog->setOptionManualLines(optionManualLines);
	optionDialog->setOptionAutoEnableDsk(optionAutoEnableDsk);
	optionDialog->setOptionAutoEnableDskScene(optionAutoEnableDskScene);
	optionDialog->setOptionIgnoreSceneList(optionIgnoreScenesList);
	//
	optionDialog->setOptionAutoTimeShow(optionAutoTimeShow);
	optionDialog->setOptionAutoTimeOff(optionAutoTimeOff);
	//
	optionDialog->setOptionEnableAutoAdvanceSceneList(optionEnableAutoAdvanceScenesList);
	optionDialog->setOptionAutoAdvanceSceneList(optionAutoAdvanceScenesList);
}


void JrYouTubeChat::loadStuff(obs_data_t *settings) {
	// hotkeys
	loadHotkey(settings, "next", hotkeyId_moveNext);
	loadHotkey(settings, "prev", hotkeyId_movePrev);
	loadHotkey(settings, "activate", hotkeyId_activateCurrent);
	loadHotkey(settings, "nextactivate", hotkeyId_moveNextAndActivate);
	loadHotkey(settings, "clear", hotkeyId_clear);
	loadHotkey(settings, "cycleTab", hotkeyId_cycleTab);
	loadHotkey(settings, "toggleAutoAdvance", hotkeyId_toggleAutoAdvance);
	loadHotkey(settings, "golast", hotkeyId_goLast);
	//
	loadHotkey(settings, "voteStart", hotkeyId_voteStart);
	loadHotkey(settings, "voteStop", hotkeyId_voteStop);
	loadHotkey(settings, "voteRestart", hotkeyId_voteRestart);
	loadHotkey(settings, "voteReopen", hotkeyId_voteReopen);
	loadHotkey(settings, "voteGolast", hotkeyId_voteGolast);

	// on main dock
	const char *ytsBuf = obs_data_get_string(settings, "youtubeid");
	youTubeIdQstrFromLastLoad = QString(ytsBuf);
	//
	// options
	ytsBuf = obs_data_get_string(settings, "chatComline");
	chatUtilityCommandLine = QString(ytsBuf);
	optionStartEmbedded = obs_data_get_bool(settings, "startEmbedded");
	optionShowEmoticons = obs_data_get_bool(settings, "showEmoticons");
	optionSetStyle = obs_data_get_bool(settings, "setStyle");
	//
	obs_data_set_default_int(settings, "fontSize", 20);
	optionFontSize = obs_data_get_int(settings, "fontSize");
	optionDefaultAvatarUrl = obs_data_get_string(settings, "defaultAvatarUrl");
	optionManualLines = obs_data_get_string(settings, "manualLines");
	optionAutoEnableDsk = obs_data_get_string(settings, "autoEnableDsk");
	optionAutoEnableDskScene = obs_data_get_string(settings, "autoEnableDskScene");
	optionIgnoreScenesList = obs_data_get_string(settings, "ignoreScenesList");
	//
	optionEnableAutoAdvanceScenesList = obs_data_get_bool(settings, "enableAutoAdvanceScenesList");
	optionAutoAdvanceScenesList = obs_data_get_string(settings, "autoAdvanceScenesList");
	//
	obs_data_set_default_int(settings, "autoTimeOff", 1000);
	obs_data_set_default_int(settings, "autoTimeShow", 5000);
	optionAutoTimeOff = obs_data_get_int(settings, "autoTimeOff");
	optionAutoTimeShow = obs_data_get_int(settings, "autoTimeShow");
}

void JrYouTubeChat::saveStuff(obs_data_t *settings) {
	// hotkeys
	saveHotkey(settings, "next", hotkeyId_moveNext);
	saveHotkey(settings, "prev", hotkeyId_movePrev);
	saveHotkey(settings, "activate", hotkeyId_activateCurrent);
	saveHotkey(settings, "nextactivate", hotkeyId_moveNextAndActivate);
	saveHotkey(settings, "clear", hotkeyId_clear);
	saveHotkey(settings, "cycleTab", hotkeyId_cycleTab);
	saveHotkey(settings, "toggleAutoAdvance", hotkeyId_toggleAutoAdvance);
	saveHotkey(settings, "golast", hotkeyId_goLast);
	saveHotkey(settings, "voteStart", hotkeyId_voteStart);
	saveHotkey(settings, "voteStop", hotkeyId_voteStop);
	saveHotkey(settings, "voteRestart", hotkeyId_voteRestart);
	saveHotkey(settings, "voteReopen", hotkeyId_voteReopen);
	saveHotkey(settings, "voteGolast", hotkeyId_voteGolast);

	auto yts = editYouTubeId->text();
	obs_data_set_string(settings, "youtubeid", yts.toUtf8().constData());
	//
	obs_data_set_string(settings, "chatComline", chatUtilityCommandLine.toUtf8().constData());
	obs_data_set_bool(settings, "startEmbedded", optionStartEmbedded);
	obs_data_set_bool(settings, "showEmoticons", optionShowEmoticons);
	obs_data_set_bool(settings, "setStyle", optionSetStyle);
	//
	obs_data_set_int(settings, "fontSize", optionFontSize);
	obs_data_set_string(settings, "defaultAvatarUrl", optionDefaultAvatarUrl.toUtf8().constData());
	obs_data_set_string(settings, "manualLines", optionManualLines.toUtf8().constData());
	obs_data_set_string(settings, "autoEnableDsk", optionAutoEnableDsk.toUtf8().constData());
	obs_data_set_string(settings, "autoEnableDskScene", optionAutoEnableDskScene.toUtf8().constData());
	obs_data_set_string(settings, "ignoreScenesList", optionIgnoreScenesList.toUtf8().constData());
	//
	obs_data_set_bool(settings, "enableAutoAdvanceScenesList", optionEnableAutoAdvanceScenesList);
	obs_data_set_string(settings, "autoAdvanceScenesList", optionAutoAdvanceScenesList.toUtf8().constData());
	//
	obs_data_set_int(settings, "autoTimeOff", optionAutoTimeOff);
	obs_data_set_int(settings, "autoTimeShow", optionAutoTimeShow);
	//
	dirtyChanges = false;
}
//---------------------------------------------------------------------------
































//---------------------------------------------------------------------------
void JrYouTubeChat::onModulePostLoad() {
	wsSetupWebsocketStuff();
	//
	// should we run this on startup?
	refillManualItems();
}

void JrYouTubeChat::onModuleUnload() {
	wsShutdownWebsocketStuff();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void JrYouTubeChat::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	JrYouTubeChat *pluginp = reinterpret_cast<JrYouTubeChat *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}

void JrYouTubeChat::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	JrYouTubeChat *pluginp = reinterpret_cast<JrYouTubeChat *>(data);
	//
	pluginp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void JrYouTubeChat::handleObsFrontendEvent(enum obs_frontend_event event) {
	switch ((int)event) {
		// handle broadcast selected
		case OBS_FRONTEND_EVENT_BROADCAST_SELECTED:
			grabVideoIdFromObsSelectedBroadcast();
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
			isStreaming = false;
			break;
		case OBS_FRONTEND_EVENT_STREAMING_STARTED:
			isStreaming = true;
			autoStartChatUtility();
			// also trigger scene change auto stuff
			onSceneChange();
			break;
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			onSceneChange();
			break;
		case OBS_FRONTEND_EVENT_TRANSITION_STOPPED:
			//onSceneChange();
			break;
	}

	// let parent handle some cases
	jrObsPlugin::handleObsFrontendEvent(event);
}


void JrYouTubeChat::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
	if (id == hotkeyId_moveNext) {
		userDoesActionStopAutoTimer();
		moveSelection(1, false);
	} else if (id == hotkeyId_movePrev) {
		userDoesActionStopAutoTimer();
		moveSelection(-1, false);
	} else if (id == hotkeyId_activateCurrent) {
		userDoesActionStopAutoTimer();
		moveSelection(0, true);
	} else if (id == hotkeyId_moveNextAndActivate) {
		userDoesActionStopAutoTimer();
		moveSelection(1, true);
	} else if (id == hotkeyId_clear) {
		userDoesActionStopAutoTimer();
		clickClearSelectedItem();
	} else if (id == hotkeyId_cycleTab) {
		//userDoesActionStopAutoTimer();
		cycleParentDockTab();
	} else if (id == hotkeyId_toggleAutoAdvance) {
		toggleAutoAdvance();
	} else if (id == hotkeyId_goLast) {
		gotoLastMessage();
	}
	else if (id == hotkeyId_voteStart) {
		userDoesActionStopAutoTimer();
		voteStartNew();
	} else if (id == hotkeyId_voteStop) {
		voteStop();
	} else if (id == hotkeyId_voteRestart) {
		userDoesActionStopAutoTimer();
		voteReopen(true);
	} else if (id == hotkeyId_voteReopen) {
		userDoesActionStopAutoTimer();
		voteReopen(false);
	} else if (id == hotkeyId_voteGolast) {
		userDoesActionStopAutoTimer();
		voteGotoLastOrCurrent();
	}

}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
JrPluginOptionsDialog* JrYouTubeChat::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrYouTubeChat::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) {
	setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog));
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrYouTubeChat::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);

	// hotkeys
	registerHotkey(ObsHotkeyCallback, this, "moveNext", hotkeyId_moveNext, "Move to next chat msg");
	registerHotkey(ObsHotkeyCallback, this, "movePrev", hotkeyId_movePrev, "Move to previous chat msg");
	registerHotkey(ObsHotkeyCallback, this, "activateCurrent", hotkeyId_activateCurrent, "Activate current chat msg");
	registerHotkey(ObsHotkeyCallback, this, "moveNextAndActivate", hotkeyId_moveNextAndActivate, "Move to next chat msg and activate");
	registerHotkey(ObsHotkeyCallback, this, "clear", hotkeyId_clear, "Clear chat msg");
	registerHotkey(ObsHotkeyCallback, this, "cycleTab", hotkeyId_cycleTab, "Cycle parent tab");
	registerHotkey(ObsHotkeyCallback, this, "toggleAutoAdvance", hotkeyId_toggleAutoAdvance, "Toggle auto advance");
	registerHotkey(ObsHotkeyCallback, this, "goLast", hotkeyId_goLast, "Goto last");
	//
	registerHotkey(ObsHotkeyCallback, this, "voteStart", hotkeyId_voteStart, "Vote start");
	registerHotkey(ObsHotkeyCallback, this, "voteStop", hotkeyId_voteStop, "Vote stop");
	registerHotkey(ObsHotkeyCallback, this, "voteRestart", hotkeyId_voteRestart, "Vote restart");
	registerHotkey(ObsHotkeyCallback, this, "voteReopen", hotkeyId_voteReopen, "Vote reopen");
	registerHotkey(ObsHotkeyCallback, this, "voteGolast", hotkeyId_voteGolast, "Vote golast");
}

void JrYouTubeChat::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);

	// hotkeys
	unRegisterHotkey(hotkeyId_moveNext);
	unRegisterHotkey(hotkeyId_movePrev);
	unRegisterHotkey(hotkeyId_activateCurrent);
	unRegisterHotkey(hotkeyId_moveNextAndActivate);
	unRegisterHotkey(hotkeyId_clear);
	unRegisterHotkey(hotkeyId_cycleTab);
	unRegisterHotkey(hotkeyId_toggleAutoAdvance);
	unRegisterHotkey(hotkeyId_goLast);
	//
	unRegisterHotkey(hotkeyId_voteStart);
	unRegisterHotkey(hotkeyId_voteStop);
	unRegisterHotkey(hotkeyId_voteRestart);
	unRegisterHotkey(hotkeyId_voteReopen);
	unRegisterHotkey(hotkeyId_voteGolast);
}

//---------------------------------------------------------------------------






























































//---------------------------------------------------------------------------
void JrYouTubeChat::showEvent(QShowEvent *) {
}

void JrYouTubeChat::hideEvent(QHideEvent *) {
}




void JrYouTubeChat::closeEvent(QCloseEvent *event) {
	if (isVisible()) {
		config_set_string(obs_frontend_get_global_config(), "JrYouTubeChat", "geometry", saveGeometry().toBase64().constData());
	}
	QWidget::closeEvent(event);
}
//---------------------------------------------------------------------------



































































//---------------------------------------------------------------------------
void JrYouTubeChat::buildUi() {
	// what does this do?
	bool deleteOnClose = moduleInstanceIsRegisteredAndAutoDeletedByObs;

	setObjectName(PLUGIN_NAME);
	setFloating(true);
	hide();

	mainLayout = new QVBoxLayout(this);

	auto* dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);
	setWidget(dockWidgetContents);

	bool optionWordWrap = false;
	if (optionSetStyle) {
		optionWordWrap = true;
	}


	// main text inputs
	//
	// label
	if (true) {
		QHBoxLayout* Hlayout = new QHBoxLayout;
		QLabel* youtubeIdLabel = new QLabel(this);
		youtubeIdLabel->setText("YouTube Video Id:");
		Hlayout->addWidget(youtubeIdLabel);
		// the text edit
		editYouTubeId = new QLineEdit(this);
		//
		Hlayout->addWidget(editYouTubeId);
		mainLayout->addLayout(Hlayout);
		// set deferred initial values
		editYouTubeId->setText(youTubeIdQstrFromLastLoad);
	}


	// bottom buttons
	bool optionAlignButtonsTop = true;
	if (true) {
		QVBoxLayout* buttonLayout = new QVBoxLayout;

		// buttons
		QHBoxLayout* buttonLayoutInner = new QHBoxLayout;
		QPushButton* chatUpdateButton = new QPushButton(QTStr("Push browser urls"));
		buttonLayoutInner->addWidget(chatUpdateButton);
		connect(chatUpdateButton, &QPushButton::clicked, [this]() { goSetBrowserChatIds(); });
		QPushButton* visitUrlButton = new QPushButton(QTStr("Open browser"));
		buttonLayoutInner->addWidget(visitUrlButton);
		connect(visitUrlButton, &QPushButton::clicked, [this]() { goOpenYtWebPage(); });
		chatUpdateButton->setStyleSheet("padding-left: 6px; padding-right: 6px;");
		visitUrlButton->setStyleSheet("padding-left: 6px; padding-right: 6px;");
		buttonLayout->addLayout(buttonLayoutInner);

		QHBoxLayout* buttonLayoutInner2 = new QHBoxLayout;
		chatUtilityLaunchButton = new QPushButton(QTStr("Launch chat"));
		buttonLayoutInner2->addWidget(chatUtilityLaunchButton);
		connect(chatUtilityLaunchButton, &QPushButton::clicked, [this]() { goLaunchChatUtility(); });
		stopUtilityButton = new QPushButton(QTStr("Kill chat"));
		buttonLayoutInner2->addWidget(stopUtilityButton);
		connect(stopUtilityButton, &QPushButton::clicked, [this]() { stopRunningProcessClick(); });
		chatUtilityLaunchButton->setStyleSheet("padding-left: 6px; padding-right: 6px;");
		stopUtilityButton->setStyleSheet("padding-left: 6px; padding-right: 6px;");
		buttonLayout->addLayout(buttonLayoutInner2);

		if (!optionAlignButtonsTop) {
			// this would BOTTOM align buttons
			mainLayout->addStretch();
		}
		mainLayout->addLayout(buttonLayout);
	}

	if (true) {
		// embedded chat log listview widget
		msgList = new QListWidget(this);
		// props
		if (optionSetStyle) {

// looks good; sometimes height is wrong
			QString qss = QString(" QListView::item {background:#646D7E; border: 1px solid #E5E4E2; margin-top: 2px; margin-bottom: 2px;} QListWidget QScrollBar{background:#000000;} QListView::item::selected {background:#659EC7;}");


			msgList->setStyleSheet(qss);

			// this creates weird gaps sometimes
			//msgList->setSpacing(3);

			// set font manually not via style sheet?
			if (true) {
				QFont font = msgList->font();
				font.setPointSize(optionFontSize);
				msgList->setFont(font);
			}
			//
			if (optionWordWrap) {
				msgList->setWordWrap(true);
			}
		}

		if (DefUseCustomItemDelegate) {
			// word wrap fix? https://forum.qt.io/topic/65588/wordwrap-true-problem-in-listview/9
			msgList->setItemDelegate(new MyQtWordWrappedListItemDelegate(this));
		}

		mainLayout->addWidget(msgList, 1);

		// wire up trigger on double click item
		connect(msgList, &QListWidget::itemActivated, [this](QListWidgetItem* item) { doMessageSelectClick(item); });
		connect(msgList, &QListWidget::itemClicked, [this](QListWidgetItem* item) { userDoesActionStopAutoTimer(); });


		// buttons after listbox
		QHBoxLayout* buttonLayoutInner2 = new QHBoxLayout;
		//
		clearOverlayButton = new QPushButton(QTStr("Clear overlay"));
		buttonLayoutInner2->addWidget(clearOverlayButton);
		connect(clearOverlayButton, &QPushButton::clicked, [this]() { clickClearSelectedItem(); });
		//
		toggleAutoAdvanceButton = new QPushButton(QTStr("Toggle Auto"));
		buttonLayoutInner2->addWidget(toggleAutoAdvanceButton);
		connect(toggleAutoAdvanceButton, &QPushButton::clicked, [this]() { toggleAutoAdvance(); });
		//
		toggleAutoAdvanceButton->setCheckable(true);
		toggleAutoAdvanceButton->setStyleSheet("QPushButtonZZZ{background-color:red;}  QPushButton:checked{background-color:\"#4c884c\"; }  QPushButton:focus{border:none;}");

		//
		mainLayout->addLayout(buttonLayoutInner2);

		chatVote.setMsgList(msgList);
	}

	// build some brush and font overrides
	if (true) {
		liFontInfo = QFontDatabase::systemFont(QFontDatabase::FixedFont);
		//liFontInfo.setPointSize(optionFontSize);
		liFontInfo.setPointSize(optionListItemInfoFontSize);
		liBrushInfo.setColor(Qt::red);
		liBrushManual.setColor(Qt::blue);
		liBrushNormal.setColor(QColor("#646D7E"));
		liBrushNormal.setStyle(Qt::SolidPattern);
	}

	// context menu for listview?
	if (true) {
		// see https://stackoverflow.com/questions/31383519/qt-rightclick-on-qlistwidget-opens-contextmenu-and-delete-item
		    // you can create the actions here, or in designer
		auto actClearList = new QAction("Clear list", this);
		connect(actClearList, &QAction::triggered, [=]() { clearMessageList(); });
		//
		auto actTestVoting = new QAction("Test voting", this);
		connect(actTestVoting, &QAction::triggered, [=]() { testVoting(); });
		//
		// and this will take care of everything else:
		msgList->setContextMenuPolicy(Qt::ActionsContextMenu);
		msgList->addActions({ actClearList, actTestVoting });

	}



	if (optionAlignButtonsTop) {
		// this would BOTTOM align buttons
		mainLayout->addStretch();
	}

	resize(260, 160);
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

	// timer
	QObject::connect(&autoTimer, &QTimer::timeout, this, &JrYouTubeChat::autoTimerTrigger);

	// initial update
	Update();

	// ATTN: move this
	if (true) {
		const char *geometry = config_get_string(obs_frontend_get_global_config(), "JrYouTubeChat", "geometry");
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

	// buttons
	updateButtonDisableds();
}
//---------------------------------------------------------------------------



























//---------------------------------------------------------------------------
void JrYouTubeChat::Update()
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
void JrYouTubeChat::Reset()
{
	Update();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void do_frontend_save(obs_data_t *save_data, bool saving, void *data) {
	JrYouTubeChat *pluginp = reinterpret_cast<JrYouTubeChat *>(data);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeChat::destructStuff() {
	prepareForDelete();
	obs_frontend_remove_save_callback(do_frontend_save, this);
	wsShutdownWebsocketStuff();
}
//---------------------------------------------------------------------------

















//---------------------------------------------------------------------------
void JrYouTubeChat::goDistribute() {
	// youtube id
	auto yts = editYouTubeId->text();
	// two things happen here, FIRST we send the video id to any chat browser sources
	sendYoutubeIdToBrowserChatSources(yts);
}



void JrYouTubeChat::goSetBrowserChatIds() {
	// stop auto timer
	userDoesActionStopAutoTimer();
	// youtube id
	auto yts = editYouTubeId->text();
	// two things happen here, FIRST we send the video id to any chat browser sources
	sendYoutubeIdToBrowserChatSources(yts);
}


void JrYouTubeChat::goLaunchChatUtility() {
	// stop auto timer
	userDoesActionStopAutoTimer();
	// youtube id
	auto yts = editYouTubeId->text();
	// two things happen here, FIRST we send the video id to any chat browser sources
	launchChatMonitorUtility(yts);
}

void JrYouTubeChat::goOpenYtWebPage() {
	// stop auto timer
	userDoesActionStopAutoTimer();
	// youtube id
	auto yts = editYouTubeId->text();
	auto url = "https://www.youtube.com/watch?v=" + yts;
	QDesktopServices::openUrl(url);
}

void JrYouTubeChat::grabVideoIdFromObsSelectedBroadcast() {
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
				//mydebug("settings: %s", obs_data_get_json(serviceSettings));
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
void JrYouTubeChat::receiveYoutubeIdSelectedSignal(QString videoid) {
	editYouTubeId->setText(videoid);
	// send it to browser chats
	sendYoutubeIdToBrowserChatSources(videoid);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeChat::launchChatMonitorUtility(QString videoid) {
	//mydebug("In JrYouTubeChat::launchChatMonitorUtility");
	//mydebug("%s", videoid.toStdString().c_str());

	// first close any running proces
	closeRunningProcess();
	waitForProcessFinish(Def_MaxSleepMsOnRestartWaitForClose);

	youTubeIdQstrUsedByChatUtil = "";
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
		mydebug("No arguments found in commandline command, not launching.");
		return;
	}
	QString program = argumentsList.takeFirst();


	// clear listbox
	clearMessageList();

	// fill initial manual items
	fillListWithManualItems();
	triggerWsFullListChangeEvent();

	// launch
	const bool constOptionStartMinimized = false;
	if (!optionStartEmbedded) {
		process.setCreateProcessArgumentsModifier(
			[constOptionStartMinimized](QProcess::CreateProcessArguments* args) {
				args->flags |= CREATE_NEW_CONSOLE;
				args->startupInfo->dwFlags &= ~STARTF_USESTDHANDLES;
				if (constOptionStartMinimized) {
					args->startupInfo->wShowWindow |= SW_SHOWMINIMIZED;
					args->startupInfo->dwFlags |= STARTF_USESHOWWINDOW;
				}
			});
	}

	if (optionStartEmbedded) {
		// catch data output
		process.setProcessChannelMode(QProcess::MergedChannels);
		QObject::connect(&process, &QProcess::readyRead, [this]() {
			this->processStdInputFromProcess();
			}
		);
	}

	process.setProgram(program);
	process.setArguments(argumentsList);

	QObject::connect(&process, &QProcess::stateChanged, [this](QProcess::ProcessState state){
		updateButtonDisableds();
	});

	if (!optionStartEmbedded) {
		process.startDetached(&chatExePid);
	}
	else {
		process.start();
		bool bretv = process.waitForStarted();
		if (!bretv) {
			std::string emsg = std::string("Error calling process.waitForStarted() on ") + comlineq.toStdString();
			mydebug("ERROR: %s",emsg.c_str());
			doMsgListAddDebugStr(QString(emsg.c_str()), true);
		}
	}

	// remember this
	youTubeIdQstrUsedByChatUtil = videoid;

	// update buttons
	updateButtonDisableds();

	// trigger
	onSceneChange();
}

void JrYouTubeChat::clearMessageList() {
	// first clear selected
	clearSelectedItemTriggerUpdate();
	// clear list contents
	msgList->clear();
	// clear and cancel any vote
	chatVote.clearAndRelease();
	// send we event
	triggerWsClearMessageListEvent();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrYouTubeChat::sendYoutubeIdToBrowserChatSources(const QString videoid) {
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
			QRegularExpression re("(.+live_chat\\?)(.*)(v=[^&\\?]*)(.*)");
			QRegularExpressionMatch match = re.match(urlq);
			if (!match.isValid()) {
				//mydebug("Match says invalid.");
			}
			if (match.hasMatch()) {
				// built replacement url
				auto pre = match.captured(1)+match.captured(2);
				auto post = match.captured(4);
				QString newUrlq = pre + QString("v=") + *videoidstrp + post;
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




//---------------------------------------------------------------------------
void JrYouTubeChat::stopRunningProcessClick() {
	// stop auto timer
	userDoesActionStopAutoTimer();
	stopRunningProcess();
}

void JrYouTubeChat::closeRunningProcess() {
	stopRunningProcess();
}


void JrYouTubeChat::stopRunningProcess() {
	if (isProcessRunning()) {
		QByteArray command("\x03\n\x04\n");
		process.write(command);
		process.closeWriteChannel();
		process.terminate();
		chatExePid = 0;
	}
}

void JrYouTubeChat::waitForProcessFinish(int maxms) {
	if (true) {
		process.waitForFinished(maxms);
		return;
	}
	else {

		process.closeWriteChannel();
		process.closeReadChannel(process.readChannel());

		long elapsedTime = 0;
		long abit = 100;
		while (isProcessRunning() && elapsedTime < maxms) {
			os_sleep_ms(abit);
			elapsedTime += abit;
		}
		process.waitForFinished(100);
	}
}



void JrYouTubeChat::prepareForDelete() {
	stopAutoAdvance();
	closeRunningProcess();
	waitForProcessFinish(Def_MaxSleepMsOnExitWaitForClose);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrYouTubeChat::processStdInputFromProcess() {
	QByteArray stdOut = process.readAll(); // process.readAllStandardOutput();
	QString qstr = QString::fromUtf8(stdOut);

	// in case we got more lines
	QStringList qstrlist = qstr.split("\r\n", Qt::SkipEmptyParts);
	for ( const auto& oneline : qstrlist  ) {
		addYouTubeItemViaJson(oneline, -1, true, true);
	}

}


bool JrYouTubeChat::isProcessRunning() {
	return process.state() == QProcess::Running;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrYouTubeChat::autoStartChatUtility() {
	// shouldn't be needed but we run it again just to make sure we are up to date
	grabVideoIdFromObsSelectedBroadcast();
	// now launch chat utility
	if (true) {
		auto yts = editYouTubeId->text();
		if (!isProcessRunning() || yts != youTubeIdQstrUsedByChatUtil) {
			goLaunchChatUtility();
		}
	}
}
//---------------------------------------------------------------------------












//---------------------------------------------------------------------------
void JrYouTubeChat::doMessageSelectClick(QListWidgetItem* item) {
	userDoesActionStopAutoTimer();
	doMessageSelect(item, true);
}


void JrYouTubeChat::doMessageSelect(QListWidgetItem* item, bool toggleOffIfOn) {
	//mydebug("User triggers doMessageSelect.");
	if (selectedListItem == item) {
		// second double click clears it
		if (toggleOffIfOn) {
			clearSelectedItem();
		}
		else {
			// nothing to do
			return;
		}
	} else {
		selectedListItem = item;
	}
	triggerWsSelectedMessageChangeEvent();
	updateDskStateAfterCheckingIgnoreList();
}



void JrYouTubeChat::clickClearSelectedItem() {
	userDoesActionStopAutoTimer();
	clearSelectedItemTriggerUpdate();
}


void JrYouTubeChat::clearSelectedItemTriggerUpdate() {
	clearSelectedItem();
	triggerWsSelectedMessageChangeEvent();
	updateDskStateAfterCheckingIgnoreList();
}

void JrYouTubeChat::userDoesActionStopAutoTimer() {
	if (optionAutoEngaged) {
		stopAutoAdvance();
	}
	// clear this
	clearAutoEnableSceneMemory();
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
void JrYouTubeChat::fillListWithManualItems() {
	// split optionManualLines into lines
	auto lines = optionManualLines.split("\n", Qt::SkipEmptyParts);
	int index = 0;
	foreach(auto oneline, lines) {
		if (oneline.startsWith("//")) {
			continue;
		}
		fillListWithManualItem(oneline, index, false);
		++index;
	}
}




void JrYouTubeChat::deleteInitialManualItems() {
	if (msgList == NULL) {
		return;
	}
	//
	int count = msgList->count();
	QListWidgetItem* p;
	for (int i = 0; i < count; ++i) {
		p = msgList->item(i);
		if (!p) {
			break;
		}
		if (p->type() != calcTypeEnumVal(JrYtWidgetListItemTypeEnum_ManuallyAdded)) {
			break;
		}
		msgList->takeItem(i);
		delete p;
		--i;
	}
}

void JrYouTubeChat::refillManualItems() {
	deleteInitialManualItems();
	fillListWithManualItems();
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrYouTubeChat::optionsFinishedChanging() {
	if (msgList) {
		// tweak font
		QFont font = msgList->font();
		font.setPointSize(optionFontSize);
		msgList->setFont(font);
	}
	jrObsPlugin::optionsFinishedChanging();
	//
	// computed values
	optionAutoTimeShowExtraLastItem = optionAutoTimeShow * 1.5;
	// stop any auto advance if on
	userDoesActionStopAutoTimer();
	// re-fill manual items
	refillManualItems();
	// re-index since we may have inserted items
	reIndexItems();
	// let clients know they need to reload list
	triggerWsFullListChangeEvent();
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrYouTubeChat::updateDskStateAfterCheckingIgnoreList() {
	bool showDsk = (selectedListItem != NULL);

	if (optionAutoEnableDsk == "") {
		// feature not set
		return;
	}

	// ATTN: now check current scene -- turn DSK OFF if its on
	if (currentSceneIsInIgnoreList()) {
		// ignore it OR turn it off?
		// i think with our new scene watching code we can just return from here and ignore this case
		bool optionIgnoreDsk = true;
		if (optionIgnoreDsk) {
			return;
		}
		// but we can at leat avoid trying to force it off if we never turned it on ourselves
		if (!ourDskLastState) {
			return;
		}
		showDsk = false;
	}
	updateDskState(showDsk);
}


void JrYouTubeChat::updateDskState(bool showDsk) {
	// try to turn the dsk on or off
	// buill json of request
	QJsonObject requestObj;
	QJsonObject vendorRequestObj;
	// add it as a json simulated item
	requestObj.insert("vendorName", "downstream-keyer");
	requestObj.insert("requestType", "dsk_select_scene");
	vendorRequestObj.insert("dsk_name", optionAutoEnableDsk.toStdString().c_str());
	if (showDsk) {
		// set scene
		vendorRequestObj.insert("scene", optionAutoEnableDskScene.toStdString().c_str());
		ourDskLastState = true;
	} else {
		// clear it
		vendorRequestObj.insert("scene", "");
		ourDskLastState = false;
	}
	requestObj.insert("requestData", vendorRequestObj);
	QJsonDocument doc(requestObj);
	QString jsonstr = doc.toJson();

	// convert json to request_data
	obs_data_t *request_data = obs_data_create_from_json(jsonstr.toStdString().c_str());

	struct obs_websocket_request_response *response = obs_websocket_call_request("CallVendorRequest", request_data);
	if (!response) {
		//mydebug("ATTN: bad reply1");;
	} else {
		//mydebug("GOT REPLY FROM dsk call.");
		//blog(LOG_WARNING, "send dsk_name %s.", optionAutoEnableDsk.toStdString().c_str());
		//blog(LOG_WARNING, "status %d.", response->status_code);
		if (response->comment) {
			//blog(LOG_WARNING, "comment %s.", response->comment);
		}
		if (response->response_data) {
			//blog(LOG_WARNING, "comment %s.", response->response_data);
		}
		//std::string reply = std::string(response->response_data);
		//blog(LOG_WARNING,"ERR: %s",(reply.c_str()));
		obs_websocket_request_response_free(response);
	}
	obs_data_release(request_data);

}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool JrYouTubeChat::moveSelection(int offset, bool trigger) {
	if (msgList->count() == 0) {
		return false;
	}

	int currentRow = msgList->currentRow();
	if (currentRow < 0 || currentRow>=msgList->count()) {
		currentRow = 0 - offset;
	}

	// move
	bool didMove = false;
	int oldCurrentRow = currentRow;
	bool foundGoodRow = false;
	QListWidgetItem* targetItemp;
	while (true) {
		currentRow += offset;
		// make sure in range
		if (currentRow >= msgList->count()) {
			currentRow = msgList->count() - 1;
			break;
		}
		if (currentRow < 0) {
			currentRow = 0;
			break;
		}
		// check that it's not INFO row
		targetItemp = msgList->item(currentRow);
		if (targetItemp->type() != calcTypeEnumVal(JrYtWidgetListItemTypeEnum_Info)) {
			// good one
			foundGoodRow = true;
			break;
		}
		if (offset == 0) {
			break;
		}
	}
	if (!foundGoodRow) {
		return false;
	}

	if (currentRow != oldCurrentRow) {
		didMove = true;
	}

	// ok we moved to a good one, select it
	msgList->setCurrentRow(currentRow, QItemSelectionModel::ClearAndSelect);

	// activate?
	if (trigger) {
		doMessageSelect(targetItemp, false);
	}

	return didMove;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeChat::cycleParentDockTab() {
	// helper util to cycle through activate tabs on parent dock
	// see https://stackoverflow.com/questions/45828478/how-to-set-current-tab-of-qtabwidget-by-name
	// see https://stackoverflow.com/questions/46613165/qt-tab-icon-when-qdockwidget-becomes-docked
	// see https://www.qtcentre.org/threads/22812-How-to-get-list-of-DockWidgets-in-QMainWindow
	// see https://bugreports.qt.io/browse/QTBUG-40913
	// see https://www.qtcentre.org/threads/21362-Setting-the-active-tab-with-tabified-docking-windows
	auto dockParent = parent();
	QList<QTabBar*> tabBars = dockParent->findChildren<QTabBar*>();
	foreach( QTabBar* bar, tabBars )
	{
		int count = bar->count();
		for (int i = 0; i < count; i++)
		{
			QString tabtitle = bar->tabText(i);
			if (tabtitle == QString(PLUGIN_LABEL)) {
				// found us, now advance tab
				int index = bar->currentIndex();
				++index;
				if (index >= bar->count()) {
					index = 0;
				}
				bar->setCurrentIndex(index);
			}
		}
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeChat::toggleAutoAdvance() {
	if (optionAutoEngaged) {
		// stop it
		stopAutoAdvance();
	}
	else {
		// start it
		startAutoAdvance();
	}
}



void JrYouTubeChat::stopAutoAdvance() {
	if (!optionAutoEngaged) {
		return;
	}
	clearSelectedItemTriggerUpdate();
	// clear flag
	optionAutoEngaged = false;
	startTimeLastItem = 0;
	// stop timer
	autoTimer.stop();
	//
	updateAutoButton();
	triggerWsStateChangeEvent_AutoToggle();
}

void JrYouTubeChat::startAutoAdvance() {
	// first stop if running
	if (optionAutoEngaged) {
		stopAutoAdvance();
	}
	// reset
	if (selectedListItem) {
		// currently showing, set timer and then hide
		autoAdvanceStage = JrYtAutoAdvanceStageEnum_Hide;
	} else {
		// not yet shown
		// do we want to start by showing the current selection? or only if new lines come in?
		if (isOnLastNonInfoRow()) {
			// when we are on last row, we just sit and wait and dont force last one on
			autoAdvanceStage = JrYtAutoAdvanceStageEnum_LastCheck;
		}
		else {
			// when user starts auto and not on last, we start by showing it
			bool bretv = moveSelection(0, true);
			autoAdvanceStage = JrYtAutoAdvanceStageEnum_Show;
		}
	}
	// set timer to advance to "next" (first) stage
	optionAutoEngaged = true;
	startTimeLastItem = 0;
	//
	updateAutoButton();
	triggerWsStateChangeEvent_AutoToggle();
	//
	setNextAutoTimer();
}

void JrYouTubeChat::autoTimerTrigger() {
	if (!optionAutoEngaged) {
		// turn it off
		return;
	}
	// ok do the next thing
	bool isLastItem = isOnLastNonInfoRow();
	if (isLastItem) {
		// we are on last item so we can't move forward
		// however we should CLEAR the selection if it was on
		// BUT we would like a LONGER time showing the LAST commment (maybe 3x as long as normal until new message comes in?); note that timer ensures we were shown for at least minimal time already
		// by default delay a bit and then look for next message to show
		autoAdvanceStage = JrYtAutoAdvanceStageEnum_LastCheck;
		if (selectedListItem) {
			// still showing last one
			// lets linger it for a some extra time
			if (startTimeLastItem==0) {
				// initialize start time
				startTimeLastItem = clock();
			}
			else {
				// has it been on long enough?
				clock_t elapsedTimeOn = clock() - startTimeLastItem;
				if (elapsedTimeOn > optionAutoTimeShowExtraLastItem) {
					// ok it's been on long enough, so turn it off
					clearSelectedItemTriggerUpdate();
					// we override this to ensure it stays off for long enough before we check for a update
					autoAdvanceStage = JrYtAutoAdvanceStageEnum_Show;
				}
			}
		}
		else {
			// it's already been turned off, nothing to do but check again in a bit
		}
	}
	else if ((autoAdvanceStage != JrYtAutoAdvanceStageEnum_Hide && !selectedListItem) || optionAutoTimeOff == 0) {
		// advance and show
		// note that we now check for && !selectedListItem so that we guarantee a time off if optionAutoTimeOff>0, no matter where we are in cycle
		bool bretv = moveSelection(1, true);
		autoAdvanceStage = JrYtAutoAdvanceStageEnum_Hide;
		startTimeLastItem = 0;
	}
	else {
		// autoAdvanceStage == 1 , so our job is to clear selection and keep it off for a bit
		clearSelectedItemTriggerUpdate();
		// drop down to move back to first display stage after some delay
		autoAdvanceStage = JrYtAutoAdvanceStageEnum_Show;
		startTimeLastItem = 0;
	}
	// and advance
	setNextAutoTimer();
}


void JrYouTubeChat::setNextAutoTimer() {
	if (autoAdvanceStage == JrYtAutoAdvanceStageEnum_LastCheck) {
		autoTimer.setInterval(optionAutoDelayBetweenLastItemChecks);
		autoTimer.start();
	} else if (autoAdvanceStage == JrYtAutoAdvanceStageEnum_Show && optionAutoTimeOff>0) {
		autoTimer.setInterval(optionAutoTimeOff);
		autoTimer.start();
	} else {
		// next stage is JrYtAutoAdvanceStageEnum_Hide, so next trigger is after we show
		autoTimer.setInterval(optionAutoTimeShow);
		autoTimer.start();
	}
}


void JrYouTubeChat::updateAutoButton() {
	// set state of Auto toggle button so it's clear to user whether auto advance is enabled
	if (toggleAutoAdvanceButton) {
		toggleAutoAdvanceButton->setChecked(optionAutoEngaged);
		toggleAutoAdvanceButton->show();
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeChat::gotoFirstMessage() {
	gotoMessageByIndex(0);
}


void JrYouTubeChat::gotoLastMessage() {
	if (!msgList) {
		return;
	}
	int rowIndex = msgList->count() - 1;
	gotoMessageByIndex(rowIndex);
}


void JrYouTubeChat::gotoMessageByIndex(int rowIndex) {
	if (rowIndex < 0) {
		return;
	}
	if (rowIndex >= msgList->count()) {
		return;
	}
	msgList->setCurrentRow(rowIndex);
}

void JrYouTubeChat::gotoNewLastIfOnNextToLast() {
	// move to last if on next to last
	if (!msgList || optionAutoEngaged) {
		// dont do this when autoengaged
		return;
	}
	int rowIndex = msgList->count() - 1;
	if (msgList->currentRow() == rowIndex-1) {
		msgList->setCurrentRow(rowIndex);
	}
}

bool JrYouTubeChat::isOnLastRow() {
	if (!msgList) {
		return false;
	}
	return (msgList->currentRow() == msgList->count() - 1);
}

bool JrYouTubeChat::isOnLastNonInfoRow() {
	int currentRow = msgList->currentRow();
	if (!msgList || currentRow<0) {
		return false;
	}
	int count = msgList->count();
	QListWidgetItem* p;
	for (int i = currentRow+1; i < count; ++i) {
		p = msgList->item(i);
		if (p->type() != calcTypeEnumVal(JrYtWidgetListItemTypeEnum_Info)) {
			return false;
		}
	}

	return true;
}


void JrYouTubeChat::gotoItemByPointer(QListWidgetItem* itemp, bool flagTrigger, bool flagToggle) {
	if (msgList == NULL) {
		return;
	}
	//
	int count = msgList->count();
	QListWidgetItem* p;
	for (int i = 0; i < count; ++i) {
		p = msgList->item(i);
		if (p == itemp) {
			// found it
			msgList->setCurrentRow(i);
			if (flagTrigger) {
				if (itemp != selectedListItem || flagToggle) {
					doMessageSelect(itemp, flagToggle);
				}
			}
			return;
		}
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeChat::updateButtonDisableds() {
	bool isRunning = !(process.state() == QProcess::NotRunning);
	bool isSelectedMessage = !(selectedListItem == NULL);
	//
	chatUtilityLaunchButton->setDisabled(isRunning);
	stopUtilityButton->setDisabled(!isRunning);
	clearOverlayButton->setDisabled(!isSelectedMessage);
//	toggleAutoAdvanceButton->setDisabled(!isSelectedMessage);
}
//---------------------------------------------------------------------------






















//---------------------------------------------------------------------------
void JrYouTubeChat::wsSetupWebsocketStuff() {
	//mydebug("In JrYouTubeChat::wsSetupWebsocketStuff.");
	vendor = obs_websocket_register_vendor("jrYoutube");
	if (!vendor) {
		warn("Failed to register websockets vendor.");
		return;
	}

	// register request clients can make of us: GET SELECTED MESSAGE
	auto event_request_cb_JrYtMessageGet = [](obs_data_t *request_data, obs_data_t *response_data,
					void *thisptr) {
		//mydebug("IN cb event_request_cb_JrYtMessageGet.");
		JrYouTubeChat* ytp = static_cast<JrYouTubeChat*>(thisptr);
		// handle callback
		ytp->checkSelectedItemStillGood();
		ytp->requestWsSelectedMessageInfoEvent(response_data, ytp->selectedListItem, ytp->getSelectedIndex(), false, false);

	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "JrYtMessageGet", event_request_cb_JrYtMessageGet, this)) {
		warn("Failed to register obs - websocket request JrYtMessageGet");
	}

	// register request clients can make of us: GET ALL MESSAGES
	auto event_request_cb_JrYtListGet = [](obs_data_t *request_data, obs_data_t *response_data,
					void *thisptr) {
		//mydebug("IN cb event_request_cb_JrYtListGet.");
		JrYouTubeChat* ytp = static_cast<JrYouTubeChat*>(thisptr);
		// handle callback
		ytp->requestWsAllMessagesInListEvent(response_data);
	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "JrYtListGet", event_request_cb_JrYtListGet, this)) {
		warn("Failed to register obs - websocket request JrYtListGet");
	}

	// register request clients can make of us: JrYtMessageSelect
	auto event_request_cb_JrYtMessageSelect = [](obs_data_t *request_data, obs_data_t *response_data,
					void *thisptr) {
		//mydebug("IN cb JrYtMessageSelect.");
		JrYouTubeChat* ytp = static_cast<JrYouTubeChat*>(thisptr);
		// handle callback
		ytp->requestWsHandleMessageSelectedByClient(request_data, response_data);
	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "JrYtMessageSelect", event_request_cb_JrYtMessageSelect, this)) {
		warn("Failed to register obs - websocket request JrYtMessageSelect");
	}


	// register request clients can make of us: JrYtMessageSelect
	auto event_request_cb_JrYtCommand = [](obs_data_t *request_data, obs_data_t *response_data,
					void *thisptr) {
		//mydebug("IN cb JrYtCommand.");
		JrYouTubeChat* ytp = static_cast<JrYouTubeChat*>(thisptr);
		// handle callback
		ytp->requestWsHandleCommandByClient(request_data, response_data);
	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "JrYtCommand", event_request_cb_JrYtCommand, this)) {
		warn("Failed to register obs - websocket request JrYtCommand");
	}


	// register request clients can make of us: JrYtMessageSelect
	auto event_request_cb_JrYtGetState = [](obs_data_t *request_data, obs_data_t *response_data,
					void *thisptr) {
		//mydebug("IN cb JrYtGetState.");
		JrYouTubeChat* ytp = static_cast<JrYouTubeChat*>(thisptr);
		// handle callback
		ytp->requestWsHandleGetState(request_data, response_data);
	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "JrYtGetState", event_request_cb_JrYtGetState, this)) {
		warn("Failed to register obs - websocket request JrYtGetState");
	}

	// register request clients can make of us: JrYtMsgStarState
	auto event_request_cb_JrYtMsgStarState = [](obs_data_t *request_data, obs_data_t *response_data,
					void *thisptr) {
		//mydebug("IN cb JrYtMsgStarState.");
		JrYouTubeChat* ytp = static_cast<JrYouTubeChat*>(thisptr);
		ytp->requestWsModifyStarState(request_data, response_data);
	};
	//
	if (!obs_websocket_vendor_register_request(vendor, "JrYtMsgStarState", event_request_cb_JrYtMsgStarState, this)) {
		warn("Failed to register obs - websocket request JrYtMsgStarState");
	}

	// register an event we send out
}


void JrYouTubeChat::wsShutdownWebsocketStuff() {
	if (vendor) {
		// concerned about crashing
		//obs_websocket_vendor_unregister_request(vendor, "JrYtMessageGet");
		vendor = NULL;
	}
}




void JrYouTubeChat::triggerWsSelectedMessageChangeEvent() {
	// after a message is selected in the listview, this function is called, it will broadcast a websocket event to any listeners

	// obs_websocket_vendor_emit_event(obs_websocket_vendor vendor, const char *event_name, obs_data_t *event_data)
	obs_data_t *event_data = obs_data_create();
	// fill it
	checkSelectedItemStillGood();
	requestWsSelectedMessageInfoEvent(event_data, selectedListItem, getSelectedIndex(), false, false);
	// emit it
	obs_websocket_vendor_emit_event(vendor, "jrYtMessageChanged", event_data);
	// release?
	obs_data_release(event_data);

	updateButtonDisableds();
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
void JrYouTubeChat::triggerWsClearMessageListEvent() {
	// empty data
	obs_data_t *event_data = obs_data_create();
	// fill it
	obs_data_set_string(event_data, "dummyvar", "dummyval");
	// emit it
	obs_websocket_vendor_emit_event(vendor, "jrYtMessageListCleared", event_data);
	// release?
	obs_data_release(event_data);
}


void JrYouTubeChat::triggerWsNewMessageAddedToListEvent(QListWidgetItem* itemp, int index) {
	// after a message is selected in the listview, this function is called, it will broadcast a websocket event to any listeners

	// obs_websocket_vendor_emit_event(obs_websocket_vendor vendor, const char *event_name, obs_data_t *event_data)
	obs_data_t *event_data = obs_data_create();
	// fill it
	requestWsSelectedMessageInfoEvent(event_data, itemp, index, true, false);
	// emit it
	obs_websocket_vendor_emit_event(vendor, "jrYtMessageAdded", event_data);
	// release?
	obs_data_release(event_data);
}


void JrYouTubeChat::triggerWsStateChangeEvent_AutoToggle() {
	// empty data
	obs_data_t *event_data = obs_data_create();
	// fill it
	obs_data_set_bool(event_data, "autoToggle", optionAutoEngaged);
	// emit it
	obs_websocket_vendor_emit_event(vendor, "jrYtStateChange", event_data);
	// release?
	obs_data_release(event_data);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrYouTubeChat::requestWsHandleMessageSelectedByClient(obs_data_t *request_data, obs_data_t* response_data) {
	// find the index of the selected item then jump to it and make it active
	int index = obs_data_get_int(request_data, "itemid");

	// turn off timer
	userDoesActionStopAutoTimer();

	if (!msgList || msgList->count() <= index) {
		return;
	}

	if (index == -1) {
		clickClearSelectedItem();
		return;
	}

	if (false) {
		const char* req = obs_data_get_json(request_data);
		mydebug("requestWsHandleMessageSelectedByClient: %s.", req);
	}

	// get it
	QListWidgetItem* targetItemp = msgList->item(index);
	// clear any existing (this should not trigger sending but means this will not toggle)?
	if (false) {
		clearSelectedItem();
	}
	// go to it
	msgList->setCurrentRow(index, QItemSelectionModel::ClearAndSelect);
	// activate it
	doMessageSelect(targetItemp, true);
}



//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrYouTubeChat::triggerWsFullListChangeEvent() {
	// empty data
	obs_data_t *event_data = obs_data_create();
	// fill it
	obs_data_set_string(event_data, "dummyvar", "dummyval");
	// emit it
	obs_websocket_vendor_emit_event(vendor, "jrYtMessageListChanged", event_data);
	// release?
	obs_data_release(event_data);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
int JrYouTubeChat::getLastListIndex() {
	if (!msgList) {
		return -1;
	}
	return msgList->count() - 1;
}

int JrYouTubeChat::getSelectedIndex() {
	return getItemIndex(selectedListItem);
}


int JrYouTubeChat::getItemIndex(QListWidgetItem* targetItemp) {
	if (targetItemp == NULL || !msgList) {
		return -1;
	}
	QListWidgetItem* itemp;
	for (int i = 0; i < msgList->count(); ++i) {
		itemp = msgList->item(i);
		if (itemp == targetItemp) {
			return i;
		}
	}
	// not found
	return -1;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool JrYouTubeChat::checkSelectedItemStillGood() {
	bool bretv = (getSelectedIndex() != -1);
	if (!bretv) {
		clearSelectedItem();
	}
	return bretv;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrYouTubeChat::requestWsHandleCommandByClient(obs_data_t *request_data, obs_data_t* response_data) {
	// find the index of the selected item then jump to it and make it active
	const char *comStr = obs_data_get_string(request_data, "command");
	if (!comStr) {
		return;
	}
	QString commandString = QString(comStr);

	// handle various commands
	if (commandString == "toggleAuto") {
		// it seems that trying to call this directly is failing to start the timer -- perhaps because it's coming in on a non-gui thread?
		QMetaObject::invokeMethod(this, [=]() { toggleAutoAdvance(); }, Qt::QueuedConnection);
	} else if (commandString == "cycleTabs") {
		cycleParentDockTab();
	} else if (commandString == "gotoLast") {
		userDoesActionStopAutoTimer();
		gotoLastMessage();
	} else if (commandString == "gotoFirst") {
		userDoesActionStopAutoTimer();
		gotoFirstMessage();
	} else if (commandString == "getStatsActive") {
		QString retstr = getStatsActive();
		obs_data_set_string(response_data, "data", retstr.toStdString().c_str());
	}  else if (commandString == "getStatsAll") {
		QString retstr = getStatsAll();
		obs_data_set_string(response_data, "data", retstr.toStdString().c_str());
	} else {
		mydebug("Unknown requestWsHandleCommandByClient: %s.", comStr);
	}
}


void JrYouTubeChat::requestWsHandleGetState(obs_data_t* request_data, obs_data_t* response_data) {
	// fill state
	obs_data_set_bool(response_data, "autoToggle", optionAutoEngaged);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
QListWidgetItem* JrYouTubeChat::getItemByIndex(int index) {
	if (index < 0 || !msgList || msgList->count() <= index) {
		return NULL;
	}
	return msgList->item(index);
}
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
void JrYouTubeChat::voteStop() {
	chatVote.stop();
	voteUpdateResults(true);
}


void JrYouTubeChat::voteReopen(bool flagClear) {
	chatVote.reopen(flagClear);
	voteUpdateResults(true);
}


void JrYouTubeChat::voteUpdateResults(bool pushChanges) {
	QString htmlResults;
	QString plainResults;
	int rowcount, maxrowwidth;
	bool isOpen;

	chatVote.buildResults(htmlResults, plainResults, rowcount, maxrowwidth, isOpen);
	//mydebug("Vote results: %s.", htmlResults.toUtf8().constData());

	// now set
	QListWidgetItem* itemp = chatVote.getVoteListItemp();
	if (!itemp) {
		return;
	}
	updateVoteItemWithTextAndLabel(itemp, htmlResults, plainResults, rowcount, maxrowwidth, isOpen);

	// push out changed results to listeners
	// ATTN: TODO this should be a generic update item message not just work if its selected
	if (pushChanges) {
		pushChangeToItem(itemp);
	}
}


void JrYouTubeChat::voteGotoLastOrCurrent() {
	auto listItemp = chatVote.getVoteListItemp();
	if (listItemp) {
		gotoItemByPointer(listItemp, true, true);
	}
}


//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrYouTubeChat::pushChangeToItem(QListWidgetItem* item) {
	// after a message is selected in the listview, this function is called, it will broadcast a websocket event to any listeners

	// obs_websocket_vendor_emit_event(obs_websocket_vendor vendor, const char *event_name, obs_data_t *event_data)
	obs_data_t *event_data = obs_data_create();
	int index = getItemIndex(item);
	requestWsSelectedMessageInfoEvent(event_data, item, index, false, true);
	// emit it
	obs_websocket_vendor_emit_event(vendor, "jrYtMessageChanged", event_data);
	// release?
	obs_data_release(event_data);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrYouTubeChat::testVoting() {
	if (false && !chatVote.isOpen()) {
		voteStartNew();
	}

	QString votingString = "Jesse: vote for pizza\nSara: vote for coffee\nNaoki: i vote for pizza\nJesse: i vote for coffee\nbing: i vote we go to b5 and then home\njonathan jang: i vote we stop the case early\nbloop bleep: vote we fight the argonautes (but this is unofficial)\nnicola: vote coffee\n";

	// test new named voter each timee
	static int anindex = 0;
	++anindex;
	votingString += "anonymous" + QString::number(anindex) + ": vote spaghetti\n";
	votingString += "generic" + QString::number(anindex) + ": vote pizza\n";

	votingString += "BAnonymous person" + QString::number(anindex) + ": vote spaghetti" + QString::number(anindex) + "\n";
	votingString += "BGeneric person" + QString::number(anindex) + ": vote pizza" + QString::number(anindex) + "\n";

	auto lines = votingString.split("\n", Qt::SkipEmptyParts);
	foreach(auto oneline, lines) {
		if (oneline.startsWith("//")) {
			continue;
		}
		fillListWithManualItem(oneline, -1, true);
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool JrYouTubeChat::currentSceneIsInIgnoreList() {
	// borrow source reference
	obs_source_t* sceneSource = obs_frontend_get_current_scene();
	if (sceneSource == NULL) {
		return false;
	}
	//
	QString sceneName = QString(obs_source_get_name(sceneSource));
	// release borrowed source reference
	obs_source_release(sceneSource);

	// ATTN: dirty imprecise inexact way to look for pattern but it's ok for our use now
	// ATTN: TODO fix
	if (optionIgnoreScenesList.contains(sceneName)) {
		return true;
	}
	return false;
}


bool JrYouTubeChat::currentSceneIsInAutoAdvanceList() {
	// borrow source reference
	obs_source_t* sceneSource = obs_frontend_get_current_scene();
	if (sceneSource == NULL) {
		return false;
	}
	//
	QString sceneName = QString(obs_source_get_name(sceneSource));
	// release borrowed source reference
	obs_source_release(sceneSource);

	// ATTN: dirty imprecise inexact way to look for pattern but it's ok for our use now
	// ATTN: TODO fix
	if (optionAutoAdvanceScenesList.contains(sceneName)) {
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// ATTN: 10/15/22 -- old note about small bug in OBS if we transition away from studio mode whiel showing a preview scene, that preview scene invokes at onscene change with non switched preview screen causing appearance of brief
// spurious scene change.. solution might be a delay between event and calling onSceneChange
void JrYouTubeChat::onSceneChange() {

	// auto ignoring dsk on certain scenes
	if (optionAutoEnableDsk != "") {
		// see if we have transitioned to or from an ignore list scene
		bool newSceneInIgnoreList = currentSceneIsInIgnoreList();
		if (newSceneInIgnoreList && !lastSceneWasInIgnoreList) {
			// going from non-ignored to ignored, we want to turn off dsk if we've turned it on
			updateDskState(false);
		}
		else if (!newSceneInIgnoreList && lastSceneWasInIgnoreList) {
			// going from ignored to non-ignored. we want to make sure we turn dsk on if it should be on
			updateDskState(selectedListItem != NULL);
		}
	// remember new status regardless of what we do
	lastSceneWasInIgnoreList = newSceneInIgnoreList;
	}

	// turning on/off auto feature
	if (optionEnableAutoAdvanceScenesList) {
		// but not if a vote is showing and open
		if (isVoteShowing() && isVoteOpen()) {
			return;
		}

		bool inAutoAdvanceScene = currentSceneIsInAutoAdvanceList();
		if (inAutoAdvanceScene && !shouldTurnOffAutoOnAutoSceneLeave) {
			// turn it on
			turnGoLastAndEnableAutoAdvance();
		}
		else if (!inAutoAdvanceScene && shouldTurnOffAutoOnAutoSceneLeave) {
			// turn it off
			//
			turnOffPreviousAutoAdvanceOnSceneLeave();
		}
	}
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
void JrYouTubeChat::turnGoLastAndEnableAutoAdvance() {
	//mydebug("In turnGoLastAndEnableAutoAdvance1.");
	if (!optionAutoEngaged) {
		//mydebug("In turnGoLastAndEnableAutoAdvance2.");
		gotoLastMessage();
		startAutoAdvance();
		shouldTurnOffAutoOnAutoSceneLeave = true;
	}
}

void JrYouTubeChat::turnOffPreviousAutoAdvanceOnSceneLeave() {
	//mydebug("In turnGoLastAndEnableAutoAdvance1 3.");
	clearAutoEnableSceneMemory();
	if (optionAutoEngaged) {
		// this will also clear the selection
		//mydebug("In turnGoLastAndEnableAutoAdvance4.");
		stopAutoAdvance();
	}
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
void JrYouTubeChat::postStartup() {
	char *config_dir = obs_module_config_path(NULL);
	if (config_dir) {
		os_mkdirs(config_dir);
		std::string baseFilePath(config_dir);
		bfree(config_dir);
		baseFilePath += "chatStats";
		chatStats.startup(baseFilePath);
	}
}
void JrYouTubeChat::initialShutdown() {
	chatStats.shutdown();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
QString JrYouTubeChat::getStatsActive() {
	return chatStats.getSessionUsersAsString();
}

QString JrYouTubeChat::getStatsAll() {
	return chatStats.getAllUsersAsString();
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool JrYouTubeChat::isVoteShowing() {
	auto listItemp = chatVote.getVoteListItemp();
	if (listItemp && listItemp == selectedListItem) {
		return true;
	}
	return false;
}

bool JrYouTubeChat::isVoteOpen() {
	return chatVote.isOpen();
}
//---------------------------------------------------------------------------

