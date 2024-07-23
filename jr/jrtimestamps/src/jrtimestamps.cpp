/*
jrtimestamps plugin for obs
what this plugin does:
Creates a _timestamps.txt file associated with every recording file, in the same directory as recordings.
This file is in YOUTUBE timestamp format, starting with 0:00 - Start of stream
And then adding a new timestamped event on the following occurences:
1. a configured hotkey is pressed
2. transition between scenes matching certain names; idea here is to record scene transitions from BREAKS to NON-BREAKS
The 0:00 OFFSET of the timestamp is taking to be the latter of:
a) recording start time
b) streaming start time
c) BROADCAST start time; this is not used by all people and can come well after streaming starts; it also relies on a new event submitted by me to obs code; if broadcaststart event is detected, newer timestamps will reset to 0:00
*/

#include "pluginInfo.hpp"
//
#include "jrtimestamps.hpp"
#include "jrtimestamps_options.hpp"
//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"


#include <obs-module.h>

#include <string>
#include <vector>


// #define JROLDSTREAMTIMESTAMP


//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define TIMER_INTERVAL 1000
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
static jrTimestamper* moduleInstance = NULL;
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load(void) {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	moduleInstance = new jrTimestamper();
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
jrTimestamper::jrTimestamper()
	: jrObsPlugin(), QObject(), timer(this)
{
	// defaults
	fillBreakScenePatterns(Def_breakPatternStringNewlined);
	//
	//
	initialStartup();
	//
	// timers
	QObject::connect(&timer, &QTimer::timeout, this, &jrTimestamper::timerTrigger);
	timer.setInterval(TIMER_INTERVAL);
	timer.start();
}

jrTimestamper::~jrTimestamper() {
	//blog(LOG_WARNING, "deleting.");

	timer.stop();
	finalizeTimestampFileIfAppropriate(true);

	finalShutdown();

	// in case this gets called by OBS so we null out the global static pointer if so
	if (moduleInstance == this) {
		moduleInstance = NULL;
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
// statics just reroute to a cast object member function call

void jrTimestamper::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	jrTimestamper *timestamper = reinterpret_cast<jrTimestamper *>(ptr);
	timestamper->handleObsFrontendEvent(event);
}

void jrTimestamper::ObsHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	jrTimestamper *timestamper = reinterpret_cast<jrTimestamper *>(data);
	//
	timestamper->handleObsHotkeyPress(id, key);
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
void jrTimestamper::handleObsFrontendEvent(enum obs_frontend_event event) {

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
			//onSceneChange();
			break;
		case OBS_FRONTEND_EVENT_TRANSITION_STOPPED:
			onSceneChange();
		break;
	}
}


void jrTimestamper::handleObsHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
	// let parent handle some cases? there are no base class cases that i know of
	jrObsPlugin::handleObsHotkeyPress(id, key);

	if (id == hotkeyId_triggerTimestamp) {
		writeTimestamp("Hotkey triggered", true);
	}
}
//---------------------------------------------------------------------------












































//---------------------------------------------------------------------------
void jrTimestamper::resetTimestampOrigin(std::string label, bool flagWithWallClockTime, std::string resetTypeStr) {
	// ATTN: new 6/7/24 we only reset on RECORDING start
	if (resetTypeStr=="recording" || timestampOrigin == 0) {
		sessionCounter = 0;
		timestampOrigin = clock();
	}
	if (resetTypeStr == "broadcasting") {
		timestampOriginBroadcasting = clock();
		#ifdef JROLDSTREAMTIMESTAMP
			broadcastingOffsetIntoStreaming = this->calcStreamingTime();
			blog(LOG_INFO, "Resetting broadcastingOffsetIntoStreaming to %lu.", (unsigned long)broadcastingOffsetIntoStreaming);
		#endif
		}


	openTimestampFileIfAppropriate();
	// log timestamp of reset event
	writeTimestamp(label, flagWithWallClockTime);
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
// ATTN: 10/15/22 -- small bug in OBS if we transition away from studio mode whiel showing a preview scene, that preview scene invokes at onscene change with non switched preview screen causing appearance of brief
// spurious scene change.. solution might be a delay between event and calling onSceneChange
void jrTimestamper::onSceneChange() {
	if (!optionEnabled) { return; }

	// borrow source reference
	obs_source_t* sceneSource = obs_frontend_get_current_scene();
	if (sceneSource == NULL) {
		return;
	}
	//
	std::string sceneName;
	sceneName = std::string(obs_source_get_name(sceneSource));
	// release borrowed source reference
	obs_source_release(sceneSource);

	//
	// blog(LOG_WARNING, "Test log entry scene change to %s.", sceneName.c_str());
	//
	recordSceneChangeIfAppropriate(sceneName);
}


void jrTimestamper::recordSceneChangeIfAppropriate(const std::string sceneName) {
	bool isBreak = isSceneNameBreakName(sceneName);

	if (isTimetampFileActive()) {
		if (optionLogAllSceneTransitions) {
			transitionedSceneFromBreakToNonBreak(lastSceneName, sceneName);
		} else {
			if (!isBreak) {
				if (lastSceneWasBreak) {
					// going from break to non-break is noteworthy
					transitionedSceneFromBreakToNonBreak(lastSceneName, sceneName);
				}
			}
		}
	}

	// remember
	lastSceneWasBreak = isBreak;
	lastSceneName = sceneName;
}


void jrTimestamper::transitionedSceneFromBreakToNonBreak(const std::string lastSceneName, const std::string sceneName) {
	char str[1024];
	bool flagWithWallClockTime = false;
	if (optionLogAllSceneTransitions) {
		sprintf(str, "%s", sceneName.c_str());
	} else if (optionLogSceneNames) {
		sprintf(str, "%s %d (%s)", sessionLabel.c_str(), ++sessionCounter, sceneName.c_str());
		flagWithWallClockTime = true;
	} else {
		sprintf(str, "%s %d", sessionLabel.c_str(), ++sessionCounter);
	}
	writeTimestamp(std::string(str), flagWithWallClockTime);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
bool jrTimestamper::openTimestampFileIfAppropriate() {
	if (timeStampFileIsOpen) {
		return true;
	}
	// filename
	std::string prefix = "";
	std::string filePathRecording = calcTimestampFilePath(prefix, "timestamp_rec.txt");
	std::string filePathStreaming = calcTimestampFilePath(prefix, "timestamp_stream.txt");
	//blog(LOG_WARNING, "Timestamp file path: %s.", filePath.c_str());
	// open file
	timestampFilepRecording = os_fopen(filePathRecording.c_str(), "w");
	timestampFilepStreaming = os_fopen(filePathStreaming.c_str(), "w");
	//
	timeStampFileIsOpen = (timestampFilepRecording!=NULL);
	//
	if (false && timeStampFileIsOpen) {
		writeInitialCommentsToTimestampFile();
	}
	return timeStampFileIsOpen;
}


void jrTimestamper::finalizeTimestampFileIfAppropriate(bool flagForce) {
	if (!timeStampFileIsOpen) {
		return;
	}
	if (!flagForce && (isStreaming || isRecording)) {
		// one of these may have ended but one is still going, so we dont end
		return;
	}
	// ATTN: TODO - close file
	fclose(timestampFilepRecording);
	fclose(timestampFilepStreaming);
	timestampFilepRecording = NULL;
	timestampFilepStreaming = NULL;
	timeStampFileIsOpen = false;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool jrTimestamper::writeTimestamp(std::string label, bool flagWithWallClockTime) {
	if (!timeStampFileIsOpen) {
		return false;
	}

	std::string parenExtra;
	if (flagWithWallClockTime) {
		parenExtra = " (" + getCurrentDateTimeAsNiceStringNoColons() + ")";
	}

	std::string timeStringRecording = getCurrentRecordingTimesOffsetAsString();
	std::string lineRecording = timeStringRecording + " - " + label + parenExtra + "\n";
	fputs(lineRecording.c_str(), timestampFilepRecording);

	std::string timeStringStreaming = getCurrentStreamingTimesOffsetAsString();
	std::string lineStreaming = timeStringStreaming + " - " + label + parenExtra + "\n";
	fputs(lineStreaming.c_str(), timestampFilepStreaming);

	if (option_FlushFileEveryLine) {
		fflush(timestampFilepRecording);
		fflush(timestampFilepStreaming);
	}
	//blog(LOG_WARNING, "Logging timestamp: %s.", line.c_str());

	blog(LOG_INFO, "ATTN: Logging LABELED STREAMING timestamp: %s.", lineStreaming.c_str());

	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
std::string jrTimestamper::getCurrentRecordingTimesOffsetAsString() {
	clock_t nowTime = clock();
	clock_t offset = nowTime - timestampOrigin;
	unsigned long secs = offset / CLOCKS_PER_SEC;

	secs = max((long)secs + optionReportAdjustSecs, 0);

	return calcSecsAsNiceTimeString(secs, optionPadLeadingZeros);
}


std::string jrTimestamper::getCurrentStreamingTimesOffsetAsString() {
	clock_t nowTime = clock();
	clock_t offset = nowTime - timestampOriginBroadcasting;
	// subtracted time spent disconnected
	offset -= disconnectedClockCount;
	unsigned long secs = offset / CLOCKS_PER_SEC;

	secs = max((long)secs + optionReportAdjustSecs, 0);

	return calcSecsAsNiceTimeString(secs, optionPadLeadingZeros);
}





std::string jrTimestamper::getCurrentStreamingTimesOffsetAsString_Previous() {
	//
	double stream_elapsed_time_sec = this->calcStreamingTime();
	if (stream_elapsed_time_sec < 0) {
		return "not streaming";
	}

	// now offset into broadcasting, since we really care about youtube time
	stream_elapsed_time_sec -= broadcastingOffsetIntoStreaming;

	//
	unsigned long secs = (long)stream_elapsed_time_sec;
	secs = max((long)secs + optionReportAdjustSecs, 0);

	return calcSecsAsNiceTimeString(secs, optionPadLeadingZeros);
}

double jrTimestamper::getObsFrameRate() {
	auto video_info = obs_video_info();
	if (obs_get_video_info(&video_info)) {
		double framerate = video_info.fps_num / video_info.fps_den;
		return framerate;
	}
	return 0;
}

double jrTimestamper::calcStreamingTime() {
	// ATTN: 6/27/24 discovered a bug; when OBS disconnects it resets the stream_frame_count, bringing it back to 0
	if (true && broadcastingOffsetIntoStreaming==0) {
		bool isStreaming = obs_frontend_streaming_active();
		if (!isStreaming) {
			//blog(LOG_INFO, "calcStreamingTime early return 1");
			return -1;
		}
	}
	auto stream_output = obs_frontend_get_streaming_output();
	if (!stream_output) {
		//blog(LOG_INFO, "calcStreamingTime early return 2");
		return -1;
	}
	auto framerate = this->getObsFrameRate();
	if (framerate == 0) {
		obs_output_release(stream_output);
		//blog(LOG_INFO, "calcStreamingTime early return 3");
		return -1;
	}

	auto stream_frame_countRaw = obs_output_get_total_frames(stream_output);
	auto droppedFrames = obs_output_get_frames_dropped(stream_output);
	int isReconnecting = obs_output_reconnecting(stream_output);
	int isActive = obs_output_active(stream_output);
	obs_output_release(stream_output);

	// track justStartedReconnecting and justFinishedReconnecting
	bool justStartedReconnecting = false;
	if (isReconnecting && !lastReconnecting) {
		justStartedReconnecting = true;
		walltime_startReconnect = time(NULL);
	}
	bool justFinishedReconnecting = false;
	if (!isReconnecting && lastReconnecting) {
		justFinishedReconnecting = true;
	}
	lastReconnecting = isReconnecting;


	// stream count modified
	auto stream_frame_count = stream_frame_countRaw;


	if (justFinishedReconnecting) {
		// we've reconnected, so now, start using the NEW values we calculated when we disconnected
		//reconnectStartFrameCount = stream_frame_count;
		reconnectStartFrameCount = disconnectFrameCount;
		// do we need to now RESET previousDroppedFrames (beacuse reconnectStartFrameCount takes it into consideration)
		if (true) {
			previousDroppedFrames = 0;
		}
		// ATTN: this should not be needed anymore (or minimal if so)
		if (true) {
			reconnectStartFrameCount -= (optionReconnectAdjustSecs * framerate);
		}
		droppingStreakFrameStart = 0;
	}





	// take into account dropped frames (current and remembered)

	if (droppedFrames < lastDroppedFrames) {
		// dropped frame count has RESET (gotten smaller)
		previousDroppedFrames += lastDroppedFrames;
		droppingStreakFrameStart = 0;
	}


	// calculate stream_frame_count by subtracting dropped frames
	stream_frame_count -= (previousDroppedFrames + droppedFrames);


	// kludge attempts to get REAL disconnect time at start of major frame dropping
	if (droppedFrames > lastDroppedFrames) {
		// frames being dropped
		// new kludge
		if (last_stream_frame_count > 0) {
			// kludge says when we drop frames, lock timestamp to previous (this is BAD for just a few dropped frames but kludge fixes lost frames on disconnect)
			long dif = (long)stream_frame_count - last_stream_frame_count;
			if (dif > 0 && dif < 60) {
				previousDroppedFrames += dif;
				stream_frame_count = last_stream_frame_count;
			}
		}
		if (!isDropping) {
			// start of dropping
			isDropping = true;
			droppingStreakFrameStart = stream_frame_count;
		}
	}
	else {
		isDropping=false;
	}

	last_stream_frame_count = stream_frame_count;

	// remember
	lastDroppedFrames = droppedFrames;


	// add reconnect offset (0 unless we had to reconnect)
	stream_frame_count += reconnectStartFrameCount;


	if (justStartedReconnecting) {
		// remember the framecount (including dropped frames and previous adjustment) but don't use it yet
		//if ((droppingStreakFrameStart > 0) && (droppingStreakFrameStart < stream_frame_count) && ((int)(stream_frame_count-droppingStreakFrameStart) < 300)) {
		long difFrameCount = (stream_frame_count - droppingStreakFrameStart);
		if (false && (droppingStreakFrameStart > 0) && (difFrameCount>0) && (difFrameCount < 300)) {
			// kludge try to use this value instead, the place where frames first started dropping
			disconnectFrameCount = droppingStreakFrameStart;
		}
		else {
			// use now framecount
			disconnectFrameCount = stream_frame_count;
		}
	}





	auto last_stream_frame_count = stream_frame_count;
	auto stream_elapsed_time_sec = stream_frame_count / framerate;

	// ATTN: test
	//blog(LOG_INFO, "ATTN: logging timestamp time, time = %f... juststartedrecon = %d, justfinishedrecon = %d;  rawframecount = %lu, framecount=%lu, recononnctstart=%lu, framerate =%lu, dropped = %lu; prevDropped = %lu; lastDropped = %lu; optionrecon = %lu; reconnecting = %d; active=%d.", (float) (stream_elapsed_time_sec - broadcastingOffsetIntoStreaming), (int)justStartedReconnecting, (int)justFinishedReconnecting, (unsigned long)stream_frame_countRaw, (unsigned long)stream_frame_count, (unsigned long)reconnectStartFrameCount, (unsigned long)framerate, (unsigned long) droppedFrames, (unsigned long)previousDroppedFrames, (unsigned long) lastDroppedFrames,  (unsigned long) optionReconnectAdjustSecs, (int) isReconnecting, (int) isActive);


	return stream_elapsed_time_sec;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrTimestamper::writeInitialCommentsToTimestampFile() {
	if (!timeStampFileIsOpen) {
		return;
	}

	std::string timeString = getCurrentDateTimeAsNiceStringNoColons();
	std::string line = std::string("// Timestamps from ") + timeString + std::string("\n");

	// ATTN: TODO write line
	fputs(line.c_str(), timestampFilepRecording);
	fputs(line.c_str(), timestampFilepStreaming);
	if (option_FlushFileEveryLine) {
		fflush(timestampFilepRecording);
		fflush(timestampFilepStreaming);
	}
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------

void jrTimestamper::loadStuff(obs_data_t *settings) {
	loadHotkey(settings, "timestampTrigger", hotkeyId_triggerTimestamp);
	//
	const char* charp = obs_data_get_string(settings, "jrTimestamps.breakPatterns");
	if (charp != NULL && strcmp(charp,"")!=0) {
		breakPatternStringNewlined = std::string(charp);
	}
	obs_data_set_default_bool(settings, "jrTimestamps.enabled", true);
	optionEnabled = obs_data_get_bool(settings, "jrTimestamps.enabled");
	obs_data_set_default_bool(settings, "jrTimestamps.logAllSceneTransitions", false);
	optionLogAllSceneTransitions = obs_data_get_bool(settings, "jrTimestamps.logAllSceneTransitions");
	//
	optionReconnectAdjustSecs = obs_data_get_int(settings, "jrTimestamps.optionReconnectAdjustSecs");
	optionReportAdjustSecs = obs_data_get_int(settings, "jrTimestamps.optionReportAdjustSecs");

	// parse breaklines
	fillBreakScenePatterns(breakPatternStringNewlined);
}

void jrTimestamper::saveStuff(obs_data_t *settings) {
	saveHotkey(settings, "timestampTrigger", hotkeyId_triggerTimestamp);
	//
	obs_data_set_string(settings, "jrTimestamps.breakPatterns", breakPatternStringNewlined.c_str());
	//
	obs_data_set_bool(settings, "jrTimestamps.enabled", optionEnabled);
	obs_data_set_bool(settings, "jrTimestamps.logAllSceneTransitions", optionLogAllSceneTransitions);
	//
	obs_data_set_int(settings, "jrTimestamps.optionReconnectAdjustSecs", optionReconnectAdjustSecs);
	obs_data_set_int(settings, "jrTimestamps.optionReportAdjustSecs", optionReportAdjustSecs);
}
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
void jrTimestamper::registerCallbacksAndHotkeys() {
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
	//
	registerHotkey(ObsHotkeyCallback, this, "timestampTrigger", hotkeyId_triggerTimestamp, "Record a timestamp entry in file");
}

void jrTimestamper::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
	//
	unRegisterHotkey(hotkeyId_triggerTimestamp);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
JrPluginOptionsDialog* jrTimestamper::createNewOptionsDialog() {
	return new OptionsDialog((QMainWindow *)obs_frontend_get_main_window(), this);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void jrTimestamper::fillBreakScenePatterns(const std::string inBreakPatternStringNewlined) {
	// record it for easy saving
	breakPatternStringNewlined = inBreakPatternStringNewlined;
	breakScenePatterns = splitString(breakPatternStringNewlined, true);
}


void jrTimestamper::setOptionKludgeAdjustments(int reconnectAdjustSecs, int reportAdjustSecs) {
	optionReconnectAdjustSecs = reconnectAdjustSecs;
	optionReportAdjustSecs = reportAdjustSecs;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
bool jrTimestamper::isSceneNameBreakName(const std::string sceneName) {
	return doesStringMatchAnyItemsInPatternList(sceneName, &breakScenePatterns);
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
void jrTimestamper::onObsExiting() {
	finalizeTimestampFileIfAppropriate(true);
	// parent call
	jrObsPlugin::onObsExiting();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrTimestamper::setSettingsOnOptionsDialog(JrPluginOptionsDialog* optionDialog) { setDerivedSettingsOnOptionsDialog(dynamic_cast<OptionsDialog*>(optionDialog)); };
//
void jrTimestamper::setDerivedSettingsOnOptionsDialog(OptionsDialog* optionDialog) {
	optionDialog->setOptionEnabled(optionEnabled);
	optionDialog->setOptionLogAllSceneTransitions(optionLogAllSceneTransitions);
	optionDialog->setBreakPatternStringNewlined(breakPatternStringNewlined);
	optionDialog->setOptionKludgeAdjustments(optionReconnectAdjustSecs, optionReportAdjustSecs);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
std::string jrTimestamper::AddTimeOffsetHintToLabel(std::string label) {
	long secsOffsetIntoRecording = 0;
	if (timestampOriginRecording > 0) {
		clock_t nowTime = clock();
		clock_t timeOffsetIntoRecording = nowTime - timestampOriginRecording;
		secsOffsetIntoRecording = timeOffsetIntoRecording / CLOCKS_PER_SEC;
	}
	if (false && secsOffsetIntoRecording > 0) {
		label += " (" + std::to_string(secsOffsetIntoRecording) + " secs into recording)";
	}
	return label;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
std::string jrTimestamper::AddVideoIdToLabel(std::string label) {
	// get global video id
	std::string videoidstr = reqVideoIdFromObsSelectedBroadcast();
	if (videoidstr != "") {
		label += " [videoid: " + videoidstr + "]";
	}
	return label;
}

std::string jrTimestamper::reqVideoIdFromObsSelectedBroadcast() {
	QString broadcastIdQstr = "";

	// ask obs for broadcast id
	if (true) {
		const char* broadcastIdStrbuf = obs_get_broadcastid_str();
		if (broadcastIdStrbuf) {
			broadcastIdQstr = QString(broadcastIdStrbuf);
		}
	}

	return broadcastIdQstr.toStdString();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrTimestamper::checkStreamingSituation() {
	#ifdef JROLDSTREAMTIMESTAMP
		calcStreamingTime();
		return;
	#endif
		updateStreamingSituation();
}


void jrTimestamper::updateStreamingSituation() {
	// ATTN: 6/27/24 discovered a bug; when OBS disconnects it resets the stream_frame_count, bringing it back to 0
	if (true && broadcastingOffsetIntoStreaming==0) {
		bool isStreaming = obs_frontend_streaming_active();
		if (!isStreaming) {
			//blog(LOG_INFO, "calcStreamingTime early return 1");
			return;
		}
	}
	auto stream_output = obs_frontend_get_streaming_output();
	if (!stream_output) {
		//blog(LOG_INFO, "calcStreamingTime early return 2");
		return;
	}

	auto droppedFrames = obs_output_get_frames_dropped(stream_output);
	int isReconnecting = obs_output_reconnecting(stream_output);
	int isActive = obs_output_active(stream_output);
	obs_output_release(stream_output);



	// track justStartedReconnecting and justFinishedReconnecting
	bool justStartedReconnecting = false;
	if (isReconnecting && !lastReconnecting) {
		justStartedReconnecting = true;
		clockStartReconnect = clock();
	}
	bool justFinishedReconnecting = false;
	if (!isReconnecting && lastReconnecting) {
		justFinishedReconnecting = true;
		clockEndReconnect = clock();
		clock_t guessStartDif = clockStartReconnect - dropStart;
		long guessStartDifSecs = guessStartDif / CLOCKS_PER_SEC;
		if (guessStartDif > 0 && guessStartDifSecs < 20) {
			// guess that the first recent drop of frames is where we REALLY lost our connection
			clockStartReconnect = dropStart;
			//blog(LOG_INFO, "For disconnect reconnect jrtimestamp, using first drop streak instead of start disconnectd (%lu sec earlier)", guessStartDifSecs);
		}
		// we have reconnected, so adjust for missing time
		auto thisDisconnectedClockCount = clockEndReconnect - clockStartReconnect;
		thisDisconnectedClockCount -= (optionReconnectAdjustSecs * CLOCKS_PER_SEC);
		// add it to our total missing time
		disconnectedClockCount += thisDisconnectedClockCount;
		float duration = thisDisconnectedClockCount / CLOCKS_PER_SEC;
		//blog(LOG_INFO, "reconnect details start = %lu  end = %lu; drop starts = %lu; dif = %lu, in secs = %lu.  optionrecond = %d; clocks_per_sec=%lu duration = %f and total = %lu", (unsigned long)clockStartReconnect, (unsigned long)clockEndReconnect, (unsigned long)dropStart, (unsigned long)guessStartDif, (unsigned long)guessStartDifSecs, (int)optionReconnectAdjustSecs, (unsigned long)CLOCKS_PER_SEC, (float)duration, (unsigned long)disconnectedClockCount);
	}
	lastReconnecting = isReconnecting;

	// kludge attempts to get REAL disconnect time at start of major frame dropping
	if (droppedFrames > lastDroppedFrames) {
		if (!isDropping) {
			// start of dropping
			isDropping = true;
			dropStart = clock();
		}
	}
	else {
		isDropping=false;
	}
	// remember
	lastDroppedFrames = droppedFrames;

	//std::string streamtimestr = getCurrentStreamingTimesOffsetAsString();
	//blog(LOG_INFO, "ATTN: logging timestamp time, streamtime = %s; disconnectedClockCount = %f;  juststartedrecon = %d, justfinishedrecon = %d; dropped = %lu; prevDropped = %lu; lastDropped = %lu; optionrecon = %d; reconnecting = %d; active=%d.", streamtimestr.c_str(), (float)disconnectedClockCount/CLOCKS_PER_SEC, (int)justStartedReconnecting, (int)justFinishedReconnecting, (unsigned long) droppedFrames, (unsigned long)previousDroppedFrames, (unsigned long) lastDroppedFrames,  (int) optionReconnectAdjustSecs, (int) isReconnecting, (int) isActive);

}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void jrTimestamper::timerTrigger() {
	auto reconnectStartFrameCountPrev = reconnectStartFrameCount;
	//
	static int wasReconnecting = 0;
	checkStreamingSituation();

	//
	if (lastReconnecting && !wasReconnecting) {
		// a reconnect has happened
		//writeTimestamp("Disconnected, attempting reconnect..", true);
	}
	else if (!lastReconnecting && wasReconnecting) {
		// a reconnect has happened
		#ifdef JROLDSTREAMTIMESTAMP
			time_t walltime_endReconnect = time(NULL);
			unsigned long timeDifSecs = walltime_endReconnect - walltime_startReconnect;
		#endif
		#ifndef JROLDSTREAMTIMESTAMP
			clock_t clockDif = clockEndReconnect - clockStartReconnect;
			unsigned long timeDifSecs = clockDif / CLOCKS_PER_SEC;
		#endif
		std::string msg = "Reconnected after " + calcSecsAsNiceTimeStringWords(timeDifSecs) + " of network outage.";
		writeTimestamp(msg, true);
	}
	//
	wasReconnecting = lastReconnecting;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrTimestamper::resetTrackedFrameCounts() {
	largestStreamingFrameCount = -1;
	reconnectStartFrameCount = 0;
	previousDroppedFrames = 0;
	lastDroppedFrames = 0;
	lastRawFrameCount = 0;
	lastReconnecting = 0;
	disconnectFrameCount = 0;
	isDropping = false;
	droppingStreakFrameStart = 0;
	last_stream_frame_count = 0;
	//
	time_t walltime_startReconnect;
	walltime_startReconnect = time(NULL);
	//
	disconnectedClockCount = 0;
	timestampOriginBroadcasting = 0;
}
//---------------------------------------------------------------------------
