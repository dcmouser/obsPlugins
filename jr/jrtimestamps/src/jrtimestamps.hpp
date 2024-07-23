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
#define Def_breakPatternStringNewlined "Title\nBreak\nIntro\n"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class OptionsDialog;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class jrTimestamper : public QObject, public jrObsPlugin {
	Q_OBJECT
protected:
	std::string lastSceneName = "";
	bool lastSceneWasBreak = false;
	int sessionCounter = 0;
	std::string sessionLabel = "Session";
	clock_t timestampOrigin = 0;
	clock_t timestampOriginRecording = 0;
	clock_t timestampOriginStreaming = 0;
	//
	clock_t timestampOriginBroadcasting = 0;
	double broadcastingOffsetIntoStreaming = 0;
	clock_t clockStartReconnect;
	clock_t clockEndReconnect;
	clock_t dropStart;
	//
	double disconnectedClockCount = 0;
	//
	long largestStreamingFrameCount = -1;
	long reconnectStartFrameCount = 0;
	long previousDroppedFrames = 0;
	long lastDroppedFrames = 0;
	long lastRawFrameCount = 0;
	int lastReconnecting = 0;
	long disconnectFrameCount = 0;
	bool isDropping;
	unsigned long droppingStreakFrameStart;
	long last_stream_frame_count;
	//
	time_t walltime_startReconnect;
	//
	bool timeStampFileIsOpen = false;
	FILE* timestampFilepRecording = NULL;
	FILE* timestampFilepStreaming = NULL;
	//
	bool isBroadcasting = false;
	bool isStreaming = false;
	bool isRecording = false;
	//
	bool optionLogSceneNames = false;
	bool optionLogAllSceneTransitions = false;
	bool optionEnabled = true;
	bool optionPadLeadingZeros = false;
	//
	bool option_FlushFileEveryLine = true;
	bool optionUseTimestampAdjust = false;
	//
	int optionReconnectAdjustSecs = 15;
	int optionReportAdjustSecs = -1;
	//
	size_t hotkeyId_triggerTimestamp = -1;
protected:
	std::string breakPatternStringNewlined;
	std::vector<std::string> breakScenePatterns;

public:
	jrTimestamper();
	~jrTimestamper();
public:
	static void ObsFrontendEvent(enum obs_frontend_event event, void* ptr);
	static void ObsHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed);
protected:
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
	virtual void handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key);

protected:
	QTimer timer;
	void timerTrigger();

protected:
	// front end events
	void BroadcastStarts() { if (!optionEnabled) { return; } isBroadcasting = true; resetTimestampOrigin(AddTimeOffsetHintToLabel("Broadcast begins"), true, "broadcasting"); };
	void BroadcastStops() { timestampEvent("Broadcast ends", true); isBroadcasting = false;  finalizeTimestampFileIfAppropriate(false); };
	void StreamingStarts() { if (!optionEnabled) { return; } resetTrackedFrameCounts();  isStreaming = true; optionUseTimestampAdjust = true;  recordTimestamp(timestampOriginStreaming);  resetTimestampOrigin(AddTimeOffsetHintToLabel(AddVideoIdToLabel("Streaming begins")), true, "streaming"); };
	void StreamingStops() { timestampEvent("Streaming ends", true); isStreaming = false; optionUseTimestampAdjust = false; zeroTimestamp(timestampOriginStreaming); isBroadcasting = false; finalizeTimestampFileIfAppropriate(false); };
	void RecordingStarts() { if (!optionEnabled) { return; } isRecording = true;  recordTimestamp(timestampOriginRecording);  if (isStreaming) { timestampEvent("Recording begins (timestamps unaffected)", true); } else { resetTimestampOrigin("Recording begins", true, "recording"); } };
	void RecordingStops() { timestampEvent("Recording ends", true); isRecording = false; zeroTimestamp(timestampOriginRecording);  finalizeTimestampFileIfAppropriate(false); };
protected:
	bool timestampEvent(std::string label, bool flagWithWallClockTime) { return writeTimestamp(label, flagWithWallClockTime); }
	bool writeTimestamp(std::string label, bool flagWithWallClockTime);
	//
	void resetTimestampOrigin(std::string label, bool flagWithWallClockTime, std::string resetTypeStr);
	void recordTimestamp(clock_t& ts) { ts = clock(); };
	void zeroTimestamp(clock_t& ts) { ts = 0; };
protected:
	void onSceneChange();
	void recordSceneChangeIfAppropriate(const std::string sceneName);
	bool isSceneNameBreakName(const std::string sceneName);
	void transitionedSceneFromBreakToNonBreak(const std::string lastSceneName, const std::string sceneName);
protected:
	bool openTimestampFileIfAppropriate();
	void finalizeTimestampFileIfAppropriate(bool flagForce);
protected:
	double getObsFrameRate();
	double calcStreamingTime();
protected:
	std::string getCurrentRecordingTimesOffsetAsString();
	std::string getCurrentStreamingTimesOffsetAsString();
	std::string getCurrentStreamingTimesOffsetAsString_Previous();
protected:
	bool isTimetampFileActive() { return timeStampFileIsOpen; };
protected:
	void writeInitialCommentsToTimestampFile();
protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };
protected:
	virtual void registerCallbacksAndHotkeys();
	virtual void unregisterCallbacksAndHotkeys();
protected:
	virtual void loadStuff(obs_data_t *settings);
	virtual void saveStuff(obs_data_t *settings);
	virtual void onObsExiting();
protected:
	virtual JrPluginOptionsDialog* createNewOptionsDialog();
	virtual void setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog);
public:
	virtual void optionsFinishedChanging() { ; };
public:
	void setDerivedSettingsOnOptionsDialog(OptionsDialog *optionDialog);
public:
	void setOptionEnabled(bool val) { optionEnabled = val; };
	void setOptionLogAllSceneTransitions(bool val) { optionLogAllSceneTransitions = val; };
	void fillBreakScenePatterns(const std::string breakPatternStringNewlined);
	void setOptionKludgeAdjustments(int reconnectAdjustSecs, int reportAdjustSecs);
protected:
	std::string AddTimeOffsetHintToLabel(std::string label);
	std::string AddVideoIdToLabel(std::string label);
	std::string reqVideoIdFromObsSelectedBroadcast();
protected:
	void resetTrackedFrameCounts();
protected:
	void checkStreamingSituation();
	void updateStreamingSituation();
};
//---------------------------------------------------------------------------


