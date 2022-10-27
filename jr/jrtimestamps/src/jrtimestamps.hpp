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
class jrTimestamper : jrObsPlugin {
protected:
	std::string lastSceneName = "";
	bool lastSceneWasBreak = false;
	int sessionCounter = 0;
	std::string sessionLabel = "Session";
	clock_t timestampOrigin = 0;
	//
	std::string timestampFileSuffix = " timestamp.txt";
	bool timeStampFileIsOpen = false;
	FILE* timestampFilep = NULL;
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
	int optionTimestampAdjustSecs = 0;
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
	// front end events
	void BroadcastStarts() { if (!optionEnabled) { return; } isBroadcasting = true; resetTimestampOrigin("Broadcast begins", true); };
	void BroadcastStops() { timestampEvent("Broadcast ends", true); isBroadcasting = false;  finalizeTimestampFileIfAppropriate(false); };
	void StreamingStarts() { if (!optionEnabled) { return; } isStreaming = true; optionUseTimestampAdjust = true;  resetTimestampOrigin("Streaming begins", true); };
	void StreamingStops() { timestampEvent("Streaming ends", true); isStreaming = false; optionUseTimestampAdjust = false; isBroadcasting = false; finalizeTimestampFileIfAppropriate(false); };
	void RecordingStarts() { if (!optionEnabled) { return; } isRecording = true; if (isStreaming) { timestampEvent("Recording begins (timestamps unaffected)", true); } else { resetTimestampOrigin("Recording begins", true); } };
	void RecordingStops() { timestampEvent("Recording ends", true); isRecording = false; finalizeTimestampFileIfAppropriate(false); };
protected:
	bool timestampEvent(std::string label, bool flagWithWallClockTime) { return writeTimestamp(label, flagWithWallClockTime); }
	bool writeTimestamp(std::string label, bool flagWithWallClockTime);
	//
	void resetTimestampOrigin(std::string label, bool flagWithWallClockTime);
protected:
	void onSceneChange();
	void recordSceneChangeIfAppropriate(const std::string sceneName);
	bool isSceneNameBreakName(const std::string sceneName);
	void transitionedSceneFromBreakToNonBreak(const std::string lastSceneName, const std::string sceneName);
protected:
	bool openTimestampFileIfAppropriate();
	void finalizeTimestampFileIfAppropriate(bool flagForce);
protected:
	std::string getCurrentTimesOffsetAsString();
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

};
//---------------------------------------------------------------------------
