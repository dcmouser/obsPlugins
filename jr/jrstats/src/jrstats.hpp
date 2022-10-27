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

#include "plugininfo.hpp"

#include "../../jrcommon/src/jrobsplugin.hpp"



//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
enum JrEnumOnAirMode {JrEnumOnAirMode_Off, JrEnumOnAirMode_Recording, JrEnumOnAirMode_Streaming, JrEnumOnAirMode_StreamRecing, JrEnumOnAirMode_Broadcasting};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define Def_breakPatternStringNewlined "Title\nBreak\nIntro\n"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// forward declarations
class QGridLayout;
class QCloseEvent;
//
class OptionsDialog;
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
class jrStats : public QDockWidget, public jrObsPlugin {
	// produces link error:
	//Q_OBJECT
protected:
	bool firstUpdate = true;
	JrEnumOnAirMode onAirMode = JrEnumOnAirMode_Off;
	int sessionSecsBeforeWarning = 55*60;
	int sessionSecsBeforeError = 60*60;
protected:
	int fontSizeNormal;
	int fontSizeHeadline;
protected:
	std::string breakPatternStringNewlined;
	std::vector<std::string> breakScenePatterns;
protected:
	QLabel *fps = nullptr;
	QLabel *cpuUsage = nullptr;
	QLabel *hddSpace = nullptr;
	QLabel *recordTimeLeft = nullptr;
	QLabel *memUsage = nullptr;

	QLabel *renderTime = nullptr;
	QLabel *skippedFrames = nullptr;
	QLabel *missedFrames = nullptr;
	
	QLabel* onAirTimeBreak = nullptr;
	QLabel* onAirTimeSession = nullptr;
	QLabel* onAirTimeGeneric = nullptr;
	QLabel* onAirTimeOff = nullptr;
	QLabel* onAirTimeBreakTypeLabel = nullptr;
	QLabel* onAirTimeSessionTypeLabel = nullptr;
	QLabel* onAirTimeGenericTypeLabel = nullptr;
	QLabel* onAirTimeOffTypeLabel = nullptr;
	QWidget* onAirBreakLayoutWidget = nullptr;
	QWidget* onAirSessionLayoutWidget = nullptr;
	QWidget* onAirGenericLayoutWidget = nullptr;
	QWidget* onAirOffLayoutWidget = nullptr;

	os_cpu_usage_info_t *cpu_info = nullptr;

	QTimer timer;
	QTimer recTimeLeft;
	uint64_t num_bytes = 0;
	std::vector<long double> bitrates;

	struct OutputLabels {
		QPointer<QLabel> name;
		QPointer<QLabel> status;
		QPointer<QLabel> droppedFrames;
		QPointer<QLabel> megabytesSent;
		QPointer<QLabel> bitrate;
		QPointer<QLabel> runTime;

		uint64_t lastBytesSent = 0;
		uint64_t lastBytesSentTime = 0;
		//
		clock_t startTime = 0;
		clock_t endTime = 0;

		int first_total = 0;
		int first_dropped = 0;

		void Update(obs_output_t *output, bool rec);
		void Reset(obs_output_t *output);

		long double kbps = 0.0l;
	};

	QList<OutputLabels> outputLabels;

	clock_t startTimeBroadcast = 0;
	clock_t stopTimeBroadcast = 0;

	clock_t startTimeOnAirBreak = 0;
	clock_t stopTimeOnAirBreak = 0;
	clock_t startTimeOnAirSession = 0;
	clock_t stopTimeOnAirSession = 0;
	clock_t startTimeOnAirGeneric = 0;
	clock_t stopTimeOnAirGeneric = 0;


public:
	jrStats(QWidget *parent = nullptr);
	~jrStats();
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
	void buildUiAddOutputLabels(QGridLayout* layout, QString name, float fontSizeMultTypeLabel, float fontSizeMultLabel);
protected:
	void Update();
protected:
	virtual void closeEvent(QCloseEvent *event) override;
public:
	static void InitializeValues();

public:
	int getFontSizeNormal() { return fontSizeNormal; };
	int getFontSizeHeadline() { return fontSizeHeadline; };
	void setFontSizeNormal(int val) { fontSizeNormal = val; }
	void setFontSizeHeadline(int val) { fontSizeHeadline = val; }

public:
	void StartRecTimeLeft();
	void ResetRecTimeLeft();
	//
	void BroadcastStarts();
	void BroadcastStops();
	void StreamingStarts();
	void StreamingStops();
	void RecordingStarts();
	void RecordingStops();
	void onAirStarts();
	void onAirStops();
	void sessionStarts();
	void sessionStops();
	void breakStarts();
	void breakStops();
	void resetSessionBreak();
	//
	bool isOnAir();
	bool isStreaming();
	bool isRecording();
	bool isBroadcasting();
	bool isInBreak();

private:
	QPointer<QObject> shortcutFilter;
private slots:
	void RecordingTimeLeft();
public slots:
	void Reset();

protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };
protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;

protected:
	static QString calcElapsedClockTimeString(clock_t startTime, clock_t endTime, bool flagForceHours, bool flagHideBlank);
protected:
	QWidget* buildOnAirLayoutWidget(std::string label, QLabel* labelp, QLabel* typeLabelp, bool flagLayoutOnAirVertical, const char* themeIdBuf, float fontSizeMultTypeLabel, float fontSizeMultLabel);
	void updateOnAirBlockTime(QLabel* labelp, QLabel* typelabelp, QWidget* layoutWidget, clock_t startTime, clock_t endTime, bool showWidget, bool showZeroTime, int secsBeforeWarning, int secsBeforeError, const char* forcedThemeStr);
protected:
	void updateOnAirMode();
protected:
	void onSceneChange();
	void updateSessionBreakOnSceneChange(bool flagForceUpdate);
	bool isSceneNameBreakName(std::string sceneName);
public:
	void fillBreakScenePatterns(const std::string inbreakPatternStringNewlined);
protected:
	QString MakeTimeLeftText(int hours, int minutes);
	QString MakeMissedFramesText(uint32_t total_lagged, uint32_t total_rendered, long double num);
};
//---------------------------------------------------------------------------

