#include "jrstats.hpp"
#include "jrstats_options.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>

#include <string>


#include <../obs-frontend-api/obs-frontend-api.h>
#include <../qt-wrappers.hpp>
/*
#include <QColorDialog>
#include <QFontDialog>
#include <QMainWindow>
#include <QMenu>
#include <QStyle>
#include <QTextList>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScreen>
*/
#include <QDateTime>


//---------------------------------------------------------------------------
// jr use
#define DefJrTestReconnectError		false
//
// normal
//#define DefJeReconnectErrorThemeId	"error"
//
// big -- use only if you have a * [themeID="bigerror"] in your style.qss
#define DefJeReconnectErrorThemeId	"bigerror"
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// for testing
#define DefAlwaysShowOnAirWidgets false
//
#define TIMER_INTERVAL 2000
#define REC_TIME_LEFT_INTERVAL 30000
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
static uint32_t first_encoded = 0xFFFFFFFF;
static uint32_t first_skipped = 0xFFFFFFFF;
static uint32_t first_rendered = 0xFFFFFFFF;
static uint32_t first_lagged = 0xFFFFFFFF;
//
#define MBYTE (1024ULL * 1024ULL)
#define GBYTE (1024ULL * 1024ULL * 1024ULL)
#define TBYTE (1024ULL * 1024ULL * 1024ULL * 1024ULL)
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
static jrStats* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = true;

bool obs_module_load() {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);

	obs_frontend_push_ui_translation(obs_module_get_string);
	//
	const auto main_window = static_cast<QMainWindow *>(obs_frontend_get_main_window());
	moduleInstance = new jrStats(main_window);
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
jrStats::jrStats(QWidget* parent)
	: jrObsPlugin(),
	QDockWidget(parent),
	cpu_info(os_cpu_usage_info_start()),
	timer(this),
	recTimeLeft(this)
{
	// defaults
	fillBreakScenePatterns(Def_breakPatternStringNewlined);
	fontSizeNormal = 15;
	fontSizeHeadline = 40;
	//
	initialStartup();

	// build the dock ui
	buildUi();
}


jrStats::~jrStats()
{
	blog(LOG_WARNING, "deleting.");

	//delete shortcutFilter;
	os_cpu_usage_info_destroy(cpu_info);
	//
	finalShutdown();

	// this can get called by OBS so we null out the global static pointer if so
	if (moduleInstance == this) {
		moduleInstance = NULL;
	}
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void jrStats::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	jrStats *plugp = reinterpret_cast<jrStats *>(ptr);
	plugp->handleObsFrontendEvent(event);
}

void jrStats::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	jrStats *plugp = reinterpret_cast<jrStats *>(data);
	//
	plugp->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void jrStats::handleObsFrontendEvent(enum obs_frontend_event event) {
	// let parent handle some cases
	jrObsPlugin::handleObsFrontendEvent(event);

	switch ((int)event) {
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
		RecordingStarts();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
		RecordingStops();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		StreamingStarts();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
		StreamingStops();
		break;
	case OBS_FRONTEND_EVENT_BROADCAST_STARTED:
		BroadcastStarts();
		break;
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
//		onSceneChange();
		break;
	case OBS_FRONTEND_EVENT_TRANSITION_STOPPED:
		onSceneChange();
		break;
	}
}


void jrStats::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
	if (id == hotkeyId_triggerStatsReset) {
		Reset();
	}
}
//---------------------------------------------------------------------------
















//---------------------------------------------------------------------------
JrPluginOptionsDialog* jrStats::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrStats::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog)); };
//
void jrStats::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	optionDialog->setBreakPatternStringNewlined(breakPatternStringNewlined);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void jrStats::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
	if (hotkeyId_triggerStatsReset==-1) hotkeyId_triggerStatsReset = obs_hotkey_register_frontend("jrStats.resetStatsTrigger", "jrStats - Reset stats", ObsHotkeyCallback, this);
}

void jrStats::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
	//
	if (hotkeyId_triggerStatsReset != -1) {
		obs_hotkey_unregister(hotkeyId_triggerStatsReset);
		hotkeyId_triggerStatsReset = -1;
	}
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void jrStats::loadStuff(obs_data_t *settings) {
	const char* charp = obs_data_get_string(settings, "jrstats.breakPatterns");
	if (charp != NULL && strcmp(charp,"")!=0) {
		breakPatternStringNewlined = std::string(charp);
	}
	// parse breaklines
	fillBreakScenePatterns(breakPatternStringNewlined);
	//
	obs_data_set_default_int(settings, "jrstats.fontSizeNormal", fontSizeNormal);
	fontSizeNormal = obs_data_get_int(settings, "jrstats.fontSizeNormal");
	obs_data_set_default_int(settings, "jjrstats.fontSizeHeadline", fontSizeHeadline);
	fontSizeHeadline = obs_data_get_int(settings, "jrstats.fontSizeHeadline");
	//
	obs_data_array_t *hotkeys = obs_data_get_array(settings, "jrstats.resetStatsTrigger");
	if (hotkeyId_triggerStatsReset!=-1 && obs_data_array_count(hotkeys)) {
		obs_hotkey_load(hotkeyId_triggerStatsReset, hotkeys);
	}
	obs_data_array_release(hotkeys);
}

void jrStats::saveStuff(obs_data_t *settings) {
	obs_data_set_string(settings, "jrstats.breakPatterns", breakPatternStringNewlined.c_str());
	obs_data_set_int(settings, "jrstats.fontSizeNormal", fontSizeNormal);
	obs_data_set_int(settings, "jrstats.fontSizeHeadline", fontSizeHeadline);
	//
	obs_data_array_t *hotkeys = obs_hotkey_save(hotkeyId_triggerStatsReset);
	obs_data_set_array(settings, "jrstats.resetStatsTrigger", hotkeys);
	obs_data_array_release(hotkeys);
}
//---------------------------------------------------------------------------




























































//---------------------------------------------------------------------------
void jrStats::showEvent(QShowEvent *) {
	timer.start(TIMER_INTERVAL);
}


void jrStats::hideEvent(QHideEvent *) {
	timer.stop();
}


void jrStats::onSceneChange() {
	if (!isOnAir()) {
		// not on air so we dont pay attention
		return;
	}
	updateSessionBreakOnSceneChange(false);
}


void jrStats::closeEvent(QCloseEvent *event) {
	if (isVisible()) {
		config_set_string(obs_frontend_get_global_config(), "jrStats", "geometry", saveGeometry().toBase64().constData());
		//config_save(obs_frontend_get_global_config());
		//config_save_safe(obs_frontend_get_global_config(), "tmp", nullptr);
	}
	QWidget::closeEvent(event);
}
//---------------------------------------------------------------------------



































































//---------------------------------------------------------------------------
void jrStats::buildUi() {
	// settings
	bool flagUseAlternateVerticalLayout = true;
	bool forceMinimumMeasuredWidths = false;

	// setting true causes crash?
	bool deleteOnClose = false;

	// ?
	setObjectName(PLUGIN_NAME);

	setFloating(true);
	hide();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	//
	QGridLayout *basicStatsLayout = new QGridLayout(this);
	QGridLayout *infoLayout = new QGridLayout(this);
	QGridLayout *outLayoutStream = new QGridLayout(this);
	QGridLayout *outLayoutRecord = new QGridLayout(this);

	auto *dockWidgetContents = new QWidget;
	dockWidgetContents->setLayout(mainLayout);
	setWidget(dockWidgetContents);

	bitrates.reserve(REC_TIME_LEFT_INTERVAL / TIMER_INTERVAL);

	int row = 0;

	float fontSizeNormalf = (float)fontSizeNormal / 10.0f;
	float fontSizeMultTypeLabel = fontSizeNormalf;
	float fontSizeMultLabel = fontSizeNormalf;
	float fontSizeMultLabel_DateTime = fontSizeMultLabel * 1.5;

	auto newStatBare = [&](QGridLayout* layout, QString name, QWidget *label, int row, int col) {
		QLabel *typeLabel = new QLabel(name, this);
		layout->addWidget(typeLabel, row, col);
		layout->addWidget(label, row, col + 1);
		bool flagSetFontSize = true;
		if (flagSetFontSize) {
			// font sizes
			if (true) {
				int newPointSize = (int)(typeLabel->font().pointSize() * fontSizeMultTypeLabel);
				//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid;}").arg(QString::number(newPointSize));
				QString qss = QString("* {font-size: %1pt;}").arg(QString::number(newPointSize));
				typeLabel->setStyleSheet(qss);
				//setThemeID(typeLabel, "");
			}
			if (true) {
				int newPointSize = (int)(label->font().pointSize() * fontSizeMultLabel);
				//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid;}").arg(QString::number(newPointSize));
				QString qss = QString("* {font-size: %1pt;}").arg(QString::number(newPointSize));
				label->setStyleSheet(qss);
				//setThemeID(labelp, "");
			}
		}
	};

	auto newStat = [&](QGridLayout* layout,const char *strLoc, QWidget *label, int row, int col) {
		std::string str = "jrstats.Stats.";
		str += strLoc;
		newStatBare(layout, QTStr(obs_module_text(str.c_str())), label, row, col);
	};



	cpuUsage = new QLabel(this);
	hddSpace = new QLabel(this);
	recordTimeLeft = new QLabel(this);
	memUsage = new QLabel(this);

	QString str = MakeTimeLeftText(999,59);
	int textWidth = recordTimeLeft->fontMetrics().boundingRect(str).width();
	if (forceMinimumMeasuredWidths) {
		recordTimeLeft->setMinimumWidth(textWidth);
	}

	fps = new QLabel(this);
	renderTime = new QLabel(this);
	skippedFrames = new QLabel(this);
	missedFrames = new QLabel(this);
	//
	frameCount = new QLabel(this);

	str = MakeMissedFramesText(9999, 99999, 99.99);
	textWidth = missedFrames->fontMetrics().boundingRect(str).width();
	if (forceMinimumMeasuredWidths) {
		missedFrames->setMinimumWidth(textWidth);
	}

	if (flagUseAlternateVerticalLayout) {
		newStat(basicStatsLayout, "AverageTimeToRender", renderTime, row, 0); row++;
		newStat(basicStatsLayout, "FrameCount", frameCount, row, 0); row++;
		newStat(basicStatsLayout, "MissedFrames", missedFrames, row, 0); row++;
		newStat(basicStatsLayout, "SkippedFrames", skippedFrames, row, 0); row++;
		newStatBare(basicStatsLayout, "FPS", fps, row, 0); row++;
	}

	newStat(basicStatsLayout, "CPUUsage", cpuUsage, row, 0); row++;
	newStat(basicStatsLayout, "MemoryUsage", memUsage, row, 0); row++;
	newStat(basicStatsLayout, "HDDSpaceAvailable", hddSpace, row, 0); row++;
	newStat(basicStatsLayout, "DiskFullIn", recordTimeLeft, row, 0); row++;


	if (!flagUseAlternateVerticalLayout) {
		row = 0;
		newStatBare(basicStatsLayout, "FPS", fps, row, 2); row++;
		newStat(basicStatsLayout, "AverageTimeToRender", renderTime, row, 2); row++;
		newStat(basicStatsLayout, "FrameCount", frameCount, row, 2); row++;
		newStat(basicStatsLayout, "MissedFrames", missedFrames, row, 2); row++;
		newStat(basicStatsLayout, "SkippedFrames", skippedFrames, row, 2); row++;
	}


	int linePaddingTop = 8;
	int linePaddingBot = 4;


	// on air layout widgets
	onAirTimeBreak = new QLabel(this);
	onAirTimeSession = new QLabel(this);
	onAirTimeGeneric = new QLabel(this);
	onAirTimeOff = new QLabel(this);
	onAirTimeBreakTypeLabel = new QLabel(this);
	onAirTimeSessionTypeLabel = new QLabel(this);
	onAirTimeGenericTypeLabel = new QLabel(this);
	onAirTimeOffTypeLabel = new QLabel(this);
	bool flagLayoutOnAirVertical = true;
	//
	float fontSizeHeadlinef = (float)fontSizeHeadline / 10.0f;
	//
	onAirOffLayoutWidget = buildOnAirLayoutWidget("OffAir", onAirTimeOff, onAirTimeOffTypeLabel, flagLayoutOnAirVertical, "", fontSizeHeadlinef * 0.75f, fontSizeHeadlinef * 0.75f);
	onAirBreakLayoutWidget = buildOnAirLayoutWidget("Break", onAirTimeBreak, onAirTimeBreakTypeLabel, flagLayoutOnAirVertical, "", fontSizeHeadlinef, fontSizeHeadlinef);
	onAirSessionLayoutWidget = buildOnAirLayoutWidget("Session", onAirTimeSession, onAirTimeSessionTypeLabel, flagLayoutOnAirVertical, "", fontSizeHeadlinef, fontSizeHeadlinef);
	onAirGenericLayoutWidget = buildOnAirLayoutWidget("OnAir", onAirTimeGeneric, onAirTimeGenericTypeLabel, flagLayoutOnAirVertical, "good", fontSizeHeadlinef, fontSizeHeadlinef);

	// bottom buttons
	QPushButton *resetButton = new QPushButton(QTStr("Reset"));
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(resetButton);
	connect(resetButton, &QPushButton::clicked, [this]() { Reset(); });



	mainLayout->addWidget(onAirOffLayoutWidget);
	mainLayout->addWidget(onAirBreakLayoutWidget);
	mainLayout->addWidget(onAirSessionLayoutWidget);
	mainLayout->addWidget(onAirGenericLayoutWidget);

	if (false) {
		QFrame* line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		mainLayout->addWidget(line);
	}

	mainLayout->addSpacing(4);
	mainLayout->addLayout(infoLayout);
	addLineSeparatorToLayout(mainLayout, linePaddingTop, linePaddingBot);

	mainLayout->addLayout(basicStatsLayout);


	// new date time layout?
	buildUiInfoLayout(infoLayout, QTStr("Streaming"), fontSizeMultTypeLabel, fontSizeMultLabel_DateTime);

	// streaming and recording stats
	buildUiAddOutputLabels(outLayoutStream, QTStr("Streaming"), fontSizeMultTypeLabel, fontSizeMultLabel);
	buildUiAddOutputLabels(outLayoutRecord, QTStr("Recording"), fontSizeMultTypeLabel, fontSizeMultLabel);

	addLineSeparatorToLayout(mainLayout, linePaddingTop, linePaddingBot);

	mainLayout->addLayout(outLayoutStream);

	addLineSeparatorToLayout(mainLayout, linePaddingTop, linePaddingBot);

	mainLayout->addLayout(outLayoutRecord);

	// stretches to keep right hand column fixed
	if (false) {
		outLayoutStream->setColumnStretch(0, 2);
		outLayoutRecord->setColumnStretch(0, 2);
		basicStatsLayout->setColumnStretch(0, 2);
	}

	mainLayout->addStretch();
	mainLayout->addLayout(buttonLayout);



	// ATTN: what is shortcutfilter?
	/*
		delete shortcutFilter;
		shortcutFilter = CreateShortcutFilter();
		installEventFilter(shortcutFilter);
	*/

	resize(640, 480);

	setMinimumSize(240, 480);

	setWindowTitle(QTStr(PLUGIN_NAME));

#ifdef __APPLE__
	setWindowIcon(
		QIcon::fromTheme("obs", QIcon(":/res/images/obs_256x256.png")));
#else
	setWindowIcon(QIcon::fromTheme("obs", QIcon(":/res/images/obs.png")));
#endif

	setWindowModality(Qt::NonModal);

	// ATTN: todo
	// this crashes?
	if (deleteOnClose) {
		setAttribute(Qt::WA_DeleteOnClose, true);
	}

	// timers
	QObject::connect(&timer, &QTimer::timeout, this, &jrStats::Update);
	timer.setInterval(TIMER_INTERVAL);
	if (isVisible())
		timer.start();

	// initial update
	Update();

	// timer
	QObject::connect(&recTimeLeft, &QTimer::timeout, this, &jrStats::RecordingTimeLeft);
	recTimeLeft.setInterval(REC_TIME_LEFT_INTERVAL);

	
	if (true) {
		const char *geometry = config_get_string(obs_frontend_get_global_config(), "jrStats", "geometry");
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

	if (obs_frontend_recording_active())
		StartRecTimeLeft();

	// add dock
	obs_frontend_add_dock(this);
}



void jrStats::buildUiAddOutputLabels(QGridLayout* layout, QString name, float fontSizeMultTypeLabel, float fontSizeMultLabel)
{
	OutputLabels ol;
	//ol.name = new QLabel(name, this);
	ol.name = NULL;
	//
	ol.status = new QLabel(this);
	ol.droppedFrames = new QLabel(this);
	ol.megabytesSent = new QLabel(this);
	ol.bitrate = new QLabel(this);
	ol.runTime = new QLabel(this);


	auto newStatBare = [&](QGridLayout* layout, QString name, QWidget *label, int row, int col, bool flagBold) {
		QLabel *typeLabel = new QLabel(name, this);
		layout->addWidget(typeLabel, row, col);
		if (label) {
			layout->addWidget(label, row, col + 1);
		}
		bool flagSetFontSize = true;
		if (flagSetFontSize) {
			// font sizes
			if (true) {
				int newPointSize = (int)(typeLabel->font().pointSize() * fontSizeMultTypeLabel);
				//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid;}").arg(QString::number(newPointSize));
				if (flagBold) {
					QString qss = QString("* {font-weight: bold; font-size: %1pt;}").arg(QString::number(newPointSize));
					typeLabel->setStyleSheet(qss);
				}
				else {
					QString qss = QString("* {font-size: %1pt;}").arg(QString::number(newPointSize));
					typeLabel->setStyleSheet(qss);
				}
			}
			if (label) {
				int newPointSize = (int)(label->font().pointSize() * fontSizeMultLabel);
				//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid;}").arg(QString::number(newPointSize));
				QString qss = QString("* {font-size: %1pt;}").arg(QString::number(newPointSize));
				label->setStyleSheet(qss);
			}
		}
	};
	auto newStat = [&](QGridLayout* layout, const char *strLoc, QWidget *label, int row, int col) {
		//std::string str = "jrstats.Stats.";
		//str += strLoc;
		std::string str = std::string(strLoc);
		newStatBare(layout, QTStr(obs_module_text(str.c_str())), label, row, col, false);
	};

	newStatBare(layout, name, NULL, 0, 0, true);
	newStat(layout, "Status", ol.status, 1, 0);
	newStat(layout, "Time", ol.runTime, 2, 0);
	newStat(layout, "Dropped", ol.droppedFrames, 3, 0);
	newStat(layout, "MbSent", ol.megabytesSent, 4, 0);
	newStat(layout, "Bitrate", ol.bitrate, 5, 0);

	// now push the labels into our list to keep track of
	outputLabels.push_back(ol);
}

//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrStats::buildUiInfoLayout(QGridLayout* layout, QString name, float fontSizeMultTypeLabel, float fontSizeMultLabel) {
	bool flagShowDateTime = true;
	bool flagShowDateTimeLabels = false;
	// test
	if (!flagShowDateTimeLabels) {
		// reduce gap between date and time
		layout->setVerticalSpacing(0);
		layout->setRowMinimumHeight(0, 0);
		layout->setRowMinimumHeight(1, 0);
	}


	auto newStatBare = [&](QGridLayout* layout, QString name, QWidget *label, int row, int col, bool flagBold, bool flagShowDateTimeLabels) {
		QLabel* typeLabel = NULL;
		if (flagShowDateTimeLabels) {
			typeLabel = new QLabel(name, this);
			layout->addWidget(typeLabel, row, col);
			++col;
		}
		if (label) {
			if (flagShowDateTimeLabels) {
				layout->addWidget(label, row, col);
			}
			else {
				layout->addWidget(label, row, col, Qt::AlignHCenter);
			}
		}
		bool flagSetFontSize = true;
		if (flagSetFontSize) {
			// font sizes
			if (typeLabel) {
				int newPointSize = (int)(typeLabel->font().pointSize() * fontSizeMultTypeLabel);
				//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid;}").arg(QString::number(newPointSize));
				if (flagBold) {
					QString qss = QString("* {font-weight: bold; font-size: %1pt;}").arg(QString::number(newPointSize));
					typeLabel->setStyleSheet(qss);
				}
				else {
					QString qss = QString("* {font-size: %1pt;}").arg(QString::number(newPointSize));
					typeLabel->setStyleSheet(qss);
				}
			}
			if (label) {
				int newPointSize = (int)(label->font().pointSize() * fontSizeMultLabel);
				//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid;}").arg(QString::number(newPointSize));
				QString qss;
				if (typeLabel) {
					qss = QString("* {font-size: %1pt;}").arg(QString::number(newPointSize));
				}
				else {
					qss = QString("* {font-size: %1pt;line-height: 40%;}").arg(QString::number(newPointSize));
				}
				label->setStyleSheet(qss);
			}
		}
	};
	auto newStat = [&](QGridLayout* layout, const char *strLoc, QWidget *label, int row, int col, bool flagShowDateTimeLabels) {
		//std::string str = "jrstats.Stats.";
		//str += strLoc;
		std::string str = std::string(strLoc);
		newStatBare(layout, QTStr(obs_module_text(str.c_str())), label, row, col, false, flagShowDateTimeLabels);
	};

	// new date and time
	int row = 0;
	if (flagShowDateTime) {
		dateLabel = new QLabel(this);
		timeLabel = new QLabel(this);
		newStat(layout, "Time", timeLabel, row, 0, flagShowDateTimeLabels); ++row;
		newStat(layout, "Date", dateLabel, row, 0, flagShowDateTimeLabels); ++row;
	}
}
//---------------------------------------------------------------------------


















































//---------------------------------------------------------------------------
void jrStats::InitializeValues()
{
	video_t *video = obs_get_video();
	first_encoded = video_output_get_total_frames(video);
	first_skipped = video_output_get_skipped_frames(video);
	first_rendered = obs_get_total_frames();
	first_lagged = obs_get_lagged_frames();
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void jrStats::Update()
{
	bool flagUseAlternateVerticalLayout = true;

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
		InitializeValues();
	}


	//
	double curFPS = obs_get_active_fps();
	double obsFPS = (double)ovi.fps_num / (double)ovi.fps_den;

	QString str = QString::number(curFPS, 'f', 2);
	fps->setText(str);

	if (curFPS < (obsFPS * 0.8))
		setThemeID(fps, "error");
	else if (curFPS < (obsFPS * 0.95))
		setThemeID(fps, "warning");
	else
		setThemeID(fps, "");



	double usage = os_cpu_usage_info_query(cpu_info);
	str = QString::number(usage, 'g', 2) + QStringLiteral("%");
	cpuUsage->setText(str);


	// note this must be freed with bfree
	const char *path = obs_frontend_get_current_record_output_path();
	num_bytes = os_get_free_disk_space(path);
	bfree((void*)path);

	QString abrv = QStringLiteral(" MB");
	long double num;

	num = (long double)num_bytes / (1024.0l * 1024.0l);
	if (num_bytes > TBYTE) {
		num /= 1024.0l * 1024.0l;
		abrv = QStringLiteral(" TB");
	} else if (num_bytes > GBYTE) {
		num /= 1024.0l;
		abrv = QStringLiteral(" GB");
	}

	str = QString::number(num, 'f', 1) + abrv;
	hddSpace->setText(str);

	if (num_bytes < GBYTE) {
		setThemeID(hddSpace, "error");
	} else if (num_bytes < (5 * GBYTE)) {
		setThemeID(hddSpace, "warning");
	} else {
		setThemeID(hddSpace, "");
	}

	/* ------------------ */

	num = (long double)os_get_proc_resident_size() / (1024.0l * 1024.0l);

	str = QString::number(num, 'f', 1) + QStringLiteral(" MB");
	memUsage->setText(str);

	/* ------------------ */

	num = (long double)obs_get_average_frame_time_ns() / 1000000.0l;

	str = QString::number(num, 'f', 1) + QStringLiteral(" ms");
	renderTime->setText(str);

	long double fpsFrameTime =
		(long double)ovi.fps_den * 1000.0l / (long double)ovi.fps_num;

	if (num > fpsFrameTime)
		setThemeID(renderTime, "error");
	else if (num > fpsFrameTime * 0.75l)
		setThemeID(renderTime, "warning");
	else
		setThemeID(renderTime, "");



	/* ------------------ */

	video_t *video = obs_get_video();
	uint32_t total_encoded = video_output_get_total_frames(video);
	uint32_t total_skipped = video_output_get_skipped_frames(video);

	if (total_encoded < first_encoded || total_skipped < first_skipped) {
		first_encoded = total_encoded;
		first_skipped = total_skipped;
	}
	total_encoded -= first_encoded;
	total_skipped -= first_skipped;

	num = total_encoded
		      ? (long double)total_skipped / (long double)total_encoded
		      : 0.0l;
	num *= 100.0l;

	if (true) {
		str = QString("%1 (%2%)")
			.arg(QString::number(total_skipped),
				QString::number(num, 'f', 1));
	} else {
		str = QString("%1 / %2 (%3%)")
			.arg(QString::number(total_skipped),
				QString::number(total_encoded),
				QString::number(num, 'f', 1));
	}
	skippedFrames->setText(str);

	if (num > 5.0l)
		setThemeID(skippedFrames, "error");
	else if (num > 1.0l)
		setThemeID(skippedFrames, "warning");
	else
		setThemeID(skippedFrames, "");

	/* ------------------ */

	uint32_t total_rendered = obs_get_total_frames();
	uint32_t total_lagged = obs_get_lagged_frames();

	if (total_rendered < first_rendered || total_lagged < first_lagged) {
		first_rendered = total_rendered;
		first_lagged = total_lagged;
	}
	total_rendered -= first_rendered;
	total_lagged -= first_lagged;


	/* ------------------ */
	// new standalone frame counter
	// total_rendered starts going up immediately just from preview; total_encoded goes up only when recording
	uint32_t fcount = total_rendered;
	if (false || (total_encoded < total_rendered && total_encoded>0)) {
		fcount = total_encoded;
	}
	str = QString("%1")
		.arg(QString::number(fcount));
	frameCount->setText(str);
	setThemeID(frameCount, "");
	/* ------------------ */


	/* ------------------ */
	// new date time
	if (true) {
		//str = QDateTime::currentDateTime().toString("HH:mm:ss");
		//str = QDateTime::currentDateTime().toString("h:mm:ss a");
		str = QDateTime::currentDateTime().toString("h:mm a");
		timeLabel->setText(str);
		setThemeID(timeLabel, "");
		//
//		str = QDateTime::currentDateTime().toString("MMM dd, yyyy");
//		str = QDateTime::currentDateTime().toString("ddd, MMM dd");
		str = QDateTime::currentDateTime().toString("dddd, MMM dd");
		dateLabel->setText(str);
		setThemeID(dateLabel, "");
	}
	/* ------------------ */



	num = total_rendered
		      ? (long double)total_lagged / (long double)total_rendered
		      : 0.0l;
	num *= 100.0l;

	str = MakeMissedFramesText(total_lagged, total_rendered, num);
	missedFrames->setText(str);

	if (num > 5.0l)
		setThemeID(missedFrames, "error");
	else if (num > 1.0l)
		setThemeID(missedFrames, "warning");
	else
		setThemeID(missedFrames, "");

	/* ------------------------------------------- */
	/* recording/streaming stats                   */

	if (true) {
		outputLabels[0].Update(strOutput, false);
		outputLabels[1].Update(recOutput, true);
		if (obs_output_active(recOutput)) {
			long double kbps = outputLabels[1].kbps;
			bitrates.push_back(kbps);
		}
	}

	// on air details
	bool isBroadcasting = (startTimeOnAirGeneric != 0 && stopTimeOnAirGeneric == 0);
	bool isOnBreak = (startTimeOnAirBreak != 0 && stopTimeOnAirBreak == 0) && isBroadcasting;
	bool showSession = (startTimeOnAirSession != 0) && isBroadcasting;
	updateOnAirBlockTime(onAirTimeOff, onAirTimeOffTypeLabel, onAirOffLayoutWidget, startTimeOnAirGeneric, stopTimeOnAirGeneric, !isBroadcasting, false, 0, 0, "");
	updateOnAirBlockTime(onAirTimeBreak, onAirTimeBreakTypeLabel, onAirBreakLayoutWidget, startTimeOnAirBreak, stopTimeOnAirBreak, isOnBreak, true, 0, 0, "");
	updateOnAirBlockTime(onAirTimeSession, onAirTimeSessionTypeLabel, onAirSessionLayoutWidget, startTimeOnAirSession, stopTimeOnAirSession, showSession, true, isOnBreak ? -1 : sessionSecsBeforeWarning, isOnBreak ? -1 : sessionSecsBeforeError, isOnBreak ? "warning" : "");
	updateOnAirBlockTime(onAirTimeGeneric, onAirTimeGenericTypeLabel, onAirGenericLayoutWidget, startTimeOnAirGeneric, stopTimeOnAirGeneric, isBroadcasting, true, 0, 0, "");
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
void jrStats::StartRecTimeLeft()
{
	if (recTimeLeft.isActive())
		ResetRecTimeLeft();

	recordTimeLeft->setText(QTStr("Calculating"));
	recTimeLeft.start();
}

void jrStats::ResetRecTimeLeft()
{
	if (recTimeLeft.isActive()) {
		bitrates.clear();
		recTimeLeft.stop();
		recordTimeLeft->setText(QTStr(""));
	}
}

void jrStats::RecordingTimeLeft()
{
	long double averageBitrate =
		accumulate(bitrates.begin(), bitrates.end(), 0.0) /
		(long double)bitrates.size();
	long double bytesPerSec = (averageBitrate / 8.0l) * 1000.0l;
	long double secondsUntilFull = (long double)num_bytes / bytesPerSec;

	bitrates.clear();

	int totalMinutes = (int)secondsUntilFull / 60;
	int minutes = totalMinutes % 60;
	int hours = totalMinutes / 60;

	QString text = MakeTimeLeftText(hours, minutes);
	recordTimeLeft->setText(text);

	if (hours < DefMinHoursLeftBeforeError) {
		setThemeID(recordTimeLeft, "error");
	} else if (hours < DefMinHoursLeftBeforeWarning) {
		setThemeID(recordTimeLeft, "warning");
	}
	else {
		setThemeID(recordTimeLeft, "");
	}
	//recordTimeLeft->setMinimumWidth(recordTimeLeft->width());
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrStats::Reset()
{
	timer.start();

	first_encoded = 0xFFFFFFFF;
	first_skipped = 0xFFFFFFFF;
	first_rendered = 0xFFFFFFFF;
	first_lagged = 0xFFFFFFFF;

	OBSOutputAutoRelease strOutput = obs_frontend_get_streaming_output();
	OBSOutputAutoRelease recOutput = obs_frontend_get_recording_output();

	outputLabels[0].Reset(strOutput);
	outputLabels[1].Reset(recOutput);

	// dont reset recording times stuff
	//	outputLabels[0].startTime = outputLabels[0].endTime = 0;
	//	outputLabels[1].startTime = outputLabels[1].endTime = 0;

	if (outputLabels[0].endTime != 0) {
		outputLabels[0].startTime = outputLabels[0].endTime = 0;
	}
	if (outputLabels[1].endTime != 0) {
		outputLabels[1].startTime = outputLabels[1].endTime = 0;
	}
	if (stopTimeOnAirBreak != 0) {
		stopTimeOnAirBreak = startTimeOnAirBreak = 0;
	}
	if (stopTimeOnAirSession != 0) {
		stopTimeOnAirSession = startTimeOnAirSession = 0;
	}
	if (stopTimeOnAirGeneric != 0) {
		stopTimeOnAirGeneric = startTimeOnAirGeneric = 0;
	}
	if (stopTimeBroadcast != 0) {
		stopTimeBroadcast = startTimeBroadcast = 0;
	}

	Update();
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void jrStats::OutputLabels::Update(obs_output_t *output, bool rec)
{
	uint64_t totalBytes = output ? obs_output_get_total_bytes(output) : 0;
	uint64_t curTime = os_gettime_ns();
	uint64_t bytesSent = totalBytes;

	if (bytesSent < lastBytesSent)
		bytesSent = 0;
	if (bytesSent == 0)
		lastBytesSent = 0;

	uint64_t bitsBetween = (bytesSent - lastBytesSent) * 8;
	long double timePassed =
		(long double)(curTime - lastBytesSentTime) / 1000000000.0l;
	kbps = (long double)bitsBetween / timePassed / 1000.0l;

	if (timePassed < 0.01l)
		kbps = 0.0l;

	QString str = QTStr("Inactive");
	QString themeID;
	bool active = output ? obs_output_active(output) : false;




	// ATTN: jr testing
	if (DefJrTestReconnectError) {
		str = QTStr("RECONNECTING");
		themeID = DefJeReconnectErrorThemeId;
	}
	else {

		if (rec) {
			if (active) {
				str = QTStr("Recording");
				themeID = "good";
			}
		}
		else {
			if (active) {
				bool reconnecting =
					output ? obs_output_reconnecting(output)
					: false;

				if (reconnecting) {
					str = QTStr("RECONNECTING");
					themeID = DefJeReconnectErrorThemeId;
					//themeID = "error";
				}
				else {
					str = QTStr("Live");
					themeID = "good";
				}
			}
		}
	}

	status->setText(str);
	setThemeID(status, themeID);

	long double num = (long double)totalBytes / (1024.0l * 1024.0l);

	megabytesSent->setText(
		QString("%1 MB").arg(QString::number(num, 'f', 1)));
	bitrate->setText(QString("%1 kb/s").arg(QString::number(kbps, 'f', 0)));

	if (!rec) {
		int total = output ? obs_output_get_total_frames(output) : 0;
		int dropped = output ? obs_output_get_frames_dropped(output)
				     : 0;

		if (total < first_total || dropped < first_dropped) {
			first_total = 0;
			first_dropped = 0;
		}

		total -= first_total;
		dropped -= first_dropped;

		num = total ? (long double)dropped / (long double)total * 100.0l
			    : 0.0l;

		if (true) {
			str = QString("%1 (%2%)")
				.arg(QString::number(dropped),
					QString::number(num, 'f', 1));
		} else {
			str = QString("%1 / %2 (%3%)")
				.arg(QString::number(dropped),
					QString::number(total),
					QString::number(num, 'f', 1));
		}

		droppedFrames->setText(str);

		if (num > 5.0l)
			setThemeID(droppedFrames, "error");
		else if (num > 1.0l)
			setThemeID(droppedFrames, "warning");
		else
			setThemeID(droppedFrames, "");
	}

	lastBytesSent = bytesSent;
	lastBytesSentTime = curTime;

	// times
	str = calcElapsedClockTimeString(startTime, endTime, true, true);
	runTime->setText(str);
	setThemeID(runTime, "");
}


void jrStats::OutputLabels::Reset(obs_output_t *output)
{
	if (!output)
		return;
	first_total = obs_output_get_total_frames(output);
	first_dropped = obs_output_get_frames_dropped(output);
}
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
void jrStats::BroadcastStarts() {
	startTimeBroadcast = clock();
	stopTimeBroadcast = 0;
	//
	onAirStarts();
	updateOnAirMode();
}

void jrStats::BroadcastStops() {
	stopTimeBroadcast = clock();
	updateOnAirMode();
}

void jrStats::StreamingStarts() {
	//blog(LOG_WARNING, "jrstats: Streaming starts");
	outputLabels[0].startTime = clock();
	outputLabels[0].endTime = 0;
	//
	onAirStarts();
	updateOnAirMode();
}

void jrStats::StreamingStops() {
	//blog(LOG_WARNING, "jrstats: Streaming stops");
	outputLabels[0].endTime = clock();
	//
	if (!isRecording()) {
		onAirStops();
	}
	BroadcastStops();
	updateOnAirMode();
}

void jrStats::RecordingStarts() {
	//blog(LOG_WARNING, "jrstats: recording starts");
	StartRecTimeLeft();
	outputLabels[1].startTime = clock();
	outputLabels[1].endTime = 0;
	//
	onAirStarts();
	updateOnAirMode();
}

void jrStats::RecordingStops() {
	//blog(LOG_WARNING, "jrstats: recording stops");
	ResetRecTimeLeft();
	outputLabels[1].endTime = clock();
	// recording does not stop generic onair if we are still STREAMING
	if (!isStreaming()) {
		onAirStops();
	}
	updateOnAirMode();
}


void jrStats::onAirStarts() {
	if (!isOnAir()) {
		resetSessionBreak();
	}
	startTimeOnAirGeneric = clock();
	stopTimeOnAirGeneric = 0;
	// session
	updateSessionBreakOnSceneChange(true);
}
void jrStats::onAirStops() {
	stopTimeOnAirGeneric = clock();
	sessionStops();
	breakStops();
}


void jrStats::sessionStarts() {
	startTimeOnAirSession = clock();
	stopTimeOnAirSession = 0;
}
void jrStats::sessionStops() {
	stopTimeOnAirSession = clock();
}
void jrStats::breakStarts() {
	startTimeOnAirBreak = clock();
	stopTimeOnAirBreak = 0;
}
void jrStats::breakStops() {
	stopTimeOnAirBreak = clock();
}

void jrStats::resetSessionBreak() {
	startTimeOnAirSession = 0;
	stopTimeOnAirSession = 0;
	startTimeOnAirBreak = 0;
	stopTimeOnAirBreak = 0;
}

bool jrStats::isStreaming() {
	return (outputLabels[0].startTime > 0 && outputLabels[0].endTime == 0);
}
bool jrStats::isRecording() {
	return (outputLabels[1].startTime > 0 && outputLabels[1].endTime == 0);
}
bool jrStats::isBroadcasting() {
	return (startTimeBroadcast > 0 && stopTimeBroadcast == 0);
}
bool jrStats::isInBreak() {
	return (startTimeOnAirBreak > 0 && stopTimeOnAirBreak == 0);
}
bool jrStats::isOnAir() {
	return (startTimeOnAirGeneric > 0 && stopTimeOnAirGeneric == 0);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
QString jrStats::calcElapsedClockTimeString(clock_t startTime, clock_t endTime, bool flagForceHours, bool flagHideBlank) {
	if (startTime == 0) {
		if (flagHideBlank) {
			return QString("");
		}
		return QString::asprintf("%d:%02d:%02d", 0, 0, 0);
	}
	if (endTime == 0) {
		endTime = clock();
	}
	unsigned long secs = (endTime - startTime) / CLOCKS_PER_SEC;

	int totalMinutes = (int)(secs / 60);
	int minutes = totalMinutes % 60;
	int hours = totalMinutes / 60;
	int secsRemainder = (int) (secs - (unsigned long)totalMinutes * 60);

	if (hours > 0 || flagForceHours) {
		return QString::asprintf("%d:%02d:%02d", hours, minutes, secsRemainder);
	}
	return QString::asprintf("%02d:%02d", minutes, secsRemainder);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
QWidget* jrStats::buildOnAirLayoutWidget(std::string label, QLabel* labelp, QLabel* typeLabelp, bool flagLayoutOnAirVertical, const char* themeIdBuf, float fontSizeMultTypeLabel, float fontSizeMultLabel) {
	auto newStatBare = [&](QGridLayout* layout, QString name, QWidget* label, QLabel* typeLabelp, int row, int col) {
		//QLabel* typeLabelp = new QLabel(name, this);
		typeLabelp->setText(name);
		if (flagLayoutOnAirVertical) {
			layout->addWidget(typeLabelp, row, col, Qt::AlignHCenter);
			layout->addWidget(label, row + 1, col, Qt::AlignHCenter);
			typeLabelp->setContentsMargins(0, 0, 0, 0);
			label->setContentsMargins(0, 0, 0, 0);

			// force fixed height based on font to make tighter spacing by ignoring gaps/padding for baseline lowercase stuff like descending "y" and superscripts
			float fscaleAdjustTypeLabel = 0.75f;
			float fscaleAdjustLabel = 0.75f;
			QRectF fm = label->fontMetrics().boundingRect("ABCDEFG");
			label->setFixedHeight(fm.height()*fontSizeMultLabel*fscaleAdjustLabel);
			fm = typeLabelp->fontMetrics().boundingRect("00:00:00");
			typeLabelp->setFixedHeight(fm.height()*fontSizeMultLabel*fscaleAdjustTypeLabel);

		}
		else {
			layout->addWidget(typeLabelp, row, col);
			layout->addWidget(label, row, col + 1);
		}
		bool flagSetFontSize = true;
		if (flagSetFontSize) {
			// font sizes
			if (true) {
				int newPointSize = (int)(typeLabelp->font().pointSize() * fontSizeMultTypeLabel);
//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid; vertical-align: bottom;}").arg(QString::number(newPointSize));
				QString qss = QString("* {font-size: %1pt; font-weight: bold;}").arg(QString::number(newPointSize));
				typeLabelp->setStyleSheet(qss);
				setThemeID(typeLabelp, themeIdBuf);
			}
			if (true) {
				int newPointSize = (int)(labelp->font().pointSize() * fontSizeMultLabel);
//				QString qss = QString("* {font-size: %1pt; font-weight: bold; border: 1px solid; vertical-align: top;}").arg(QString::number(newPointSize));
				QString qss = QString("* {font-size: %1pt; font-weight: bold;}").arg(QString::number(newPointSize));
				labelp->setStyleSheet(qss);
				setThemeID(labelp, "");
			}
			// default hidden
			labelp->setVisible(false);
		}
	};

	auto newStat = [&](QGridLayout* layout, const char *strLoc, QWidget *label, QLabel* typeLabelp, int row, int col) {
		std::string str = "jrstats.Stats.";
		str += strLoc;
		newStatBare(layout, QTStr(obs_module_text(str.c_str())), label, typeLabelp, row, col);
	};

	QGridLayout *onAirLayout = new QGridLayout(this);

	newStat(onAirLayout, label.c_str(), labelp, typeLabelp, 0, 0);

	// and some bottom margin spacing, so that we can hide and show these layouts and have good spacing between them and other layouts if visible or not
	int cmleft, cmtop, cmright, cmbottom;
	onAirLayout->getContentsMargins(&cmleft, &cmtop, &cmright, &cmbottom);
	int cmbottomNew = cmbottom * 2 + 10;
	onAirLayout->setContentsMargins(0,0,0,0);
	onAirLayout->setSpacing(0);
	// wrap layouts in widget
	QWidget* onAirLayoutWidget = new QWidget(this);
	onAirLayoutWidget->setLayout(onAirLayout);

//	QSpacerItem* qspacer = new QSpacerItem(1,12);
//	onAirLayout->addItem(qspacer,2,0);

	int paddingTop = 12;
	int paddingBot = 1;
	addLineSeparatorToLayout(onAirLayout, paddingTop, paddingBot);


	// default hidden
	onAirLayoutWidget->setVisible(false);
	// return it
	return onAirLayoutWidget;
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
void jrStats::updateOnAirBlockTime(QLabel* labelp, QLabel* typelabelp, QWidget* layoutWidget, clock_t startTime, clock_t endTime, bool showWidget, bool showZeroTime, int secsBeforeWarning, int secsBeforeError, const char* forcedThemeStr) {

	// test
	if (DefAlwaysShowOnAirWidgets) {
		showWidget = true;
		showZeroTime = true;
	}

	if (!showWidget) {
		if (layoutWidget->isVisible()) {
			layoutWidget->setVisible(false);
		}
		return;
	}
	if (!showZeroTime && startTime == 0) {
		// hide it
		if (labelp->isVisible()) {
			labelp->setVisible(false);
		}
	} else {
		// show it
		char themeIdStr[16];
		strcpy(themeIdStr, forcedThemeStr);
		QString str = calcElapsedClockTimeString(startTime, endTime, false, !showZeroTime);
		labelp->setText(str);
		if (strcmp(forcedThemeStr, "") == 0 && (secsBeforeWarning > 0 || secsBeforeError>0) && startTime > 0) {
			if (endTime == 0) {
				endTime = clock();
			}
			int secs = (endTime - startTime) / CLOCKS_PER_SEC;
			if (secsBeforeError>0 && secs >= secsBeforeError) {
				strcpy(themeIdStr, "error");
			} else if (secsBeforeWarning>0 && secs >= secsBeforeWarning) {
				strcpy(themeIdStr, "warning");
			} else {
				strcpy(themeIdStr, "good");
			}
		}
		// we check for !=0 here so we can clear in some cases
		if (true || secsBeforeWarning != 0 || secsBeforeError != 0 || strcmp(forcedThemeStr,"")!=-0) {
			setThemeID(labelp, themeIdStr);
			setThemeID(typelabelp, themeIdStr);
		}
		if (!labelp->isVisible()) {
			labelp->setVisible(true);
		}
	}
	//
	if (showWidget) {
		if (!layoutWidget->isVisible())
			layoutWidget->setVisible(true);
	};
}



void jrStats::updateOnAirMode() {
	// update state
	// hierarchy
	if (isBroadcasting()) {
		onAirMode = JrEnumOnAirMode_Broadcasting;
	} else if (isStreaming() && isRecording()) {
		onAirMode = JrEnumOnAirMode_StreamRecing;
	} else if (isStreaming()) {
		onAirMode = JrEnumOnAirMode_Streaming;
	} else if (isRecording()) {
		onAirMode = JrEnumOnAirMode_Recording;
	} else {
		onAirMode = JrEnumOnAirMode_Off;
	}

	// force label update
	QString onAirLabelText;
	if (onAirMode == JrEnumOnAirMode_Off) {
		onAirLabelText = "STOPPING";
	} else if (onAirMode == JrEnumOnAirMode_Recording) {
		onAirLabelText = "REC";
	} else if (onAirMode == JrEnumOnAirMode_Streaming) {
		onAirLabelText = "STREAM";
	} else if (onAirMode == JrEnumOnAirMode_StreamRecing) {
		onAirLabelText = "STR-REC";
	} else if (onAirMode == JrEnumOnAirMode_Broadcasting) {
		onAirLabelText = "ON AIR";
	}
	onAirTimeGenericTypeLabel->setText(onAirLabelText);
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
void jrStats::updateSessionBreakOnSceneChange(bool flagForceUpdate) {

	// borrow source reference
	obs_source_t* sceneSource = obs_frontend_get_current_scene();

	if (sceneSource == NULL) {
		if (flagForceUpdate) {
			sessionStarts();
		}
		return;
	}

	std::string sceneName;
	sceneName = std::string(obs_source_get_name(sceneSource));

	// release borrowed source reference
	obs_source_release(sceneSource);

	bool wasInBreak = isInBreak();
	bool isBreakScene = isSceneNameBreakName(sceneName);
	if (wasInBreak == isBreakScene && !flagForceUpdate) {
		// nothing changed;
		return;
	}
	// changing to and from break
	if (isBreakScene) {
		sessionStops();
		breakStarts();
	}
	else {
		breakStops();
		sessionStarts();
	}
}
//---------------------------------------------------------------------------






















//---------------------------------------------------------------------------
void jrStats::fillBreakScenePatterns(const std::string inBreakPatternStringNewlined) {
	// record it for easy saving
	breakPatternStringNewlined = inBreakPatternStringNewlined;
	breakScenePatterns = splitString(breakPatternStringNewlined, true);
}


bool jrStats::isSceneNameBreakName(const std::string sceneName) {
	return doesStringMatchAnyItemsInPatternList(sceneName, &breakScenePatterns);
}
//---------------------------------------------------------------------------











































//---------------------------------------------------------------------------
QString jrStats::MakeTimeLeftText(int hours, int minutes)
{
	//return QString::asprintf("%d %s, %d %s", hours, QT_TO_UTF8(QTStr("Hours")), minutes, QT_TO_UTF8(QTStr("Minutes")));
	return QString::asprintf("%d %s", hours, QT_TO_UTF8(QTStr("hr")));
}

QString jrStats::MakeMissedFramesText(uint32_t total_lagged, uint32_t total_rendered, long double num)
{
	if (true) {
		return QString("%1 (%2%)")
			.arg(QString::number(total_lagged),
				QString::number(num, 'f', 1));
	} else {
		return QString("%1 / %2 (%3%)")
			.arg(QString::number(total_lagged),
				QString::number(total_rendered),
				QString::number(num, 'f', 1));
	}
}
//---------------------------------------------------------------------------

