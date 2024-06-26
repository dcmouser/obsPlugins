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





//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
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
jrTimestamper::jrTimestamper() {
	// defaults
	fillBreakScenePatterns(Def_breakPatternStringNewlined);
	//
	initialStartup();
}

jrTimestamper::~jrTimestamper() {
	//blog(LOG_WARNING, "deleting.");

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
		broadcastingOffsetIntoStreaming = this->calcStreamingTime();
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
	char str[255];
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
		parenExtra = " (" + getCurrentDateTimeAsNiceString() + ")";
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

	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
std::string jrTimestamper::getCurrentRecordingTimesOffsetAsString() {
	clock_t nowTime = clock();
	clock_t offset = nowTime - timestampOrigin;
	unsigned long secs = offset / CLOCKS_PER_SEC;

	if (optionTimestampAdjustSecs > 0 ||  (optionTimestampAdjustSecs<0 && secs > (unsigned long) (- 1 * optionTimestampAdjustSecs))) {
		secs += optionTimestampAdjustSecs;
	}

	return calcSecsAsNiceTimeString(secs, optionPadLeadingZeros);
}





std::string jrTimestamper::getCurrentStreamingTimesOffsetAsString() {
	//
	double stream_elapsed_time_sec = this->calcStreamingTime();
	if (stream_elapsed_time_sec < 0) {
		return "not streaming";
	}

	// now offset into broadcasting, since we really care about youtube time
	stream_elapsed_time_sec -= broadcastingOffsetIntoStreaming;

	//
	unsigned long secs = (long)stream_elapsed_time_sec;
	//
	if (optionTimestampAdjustSecs > 0 ||  (optionTimestampAdjustSecs<0 && secs > (unsigned long) (- 1 * optionTimestampAdjustSecs))) {
		secs += optionTimestampAdjustSecs;
	}

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
	if (true && broadcastingOffsetIntoStreaming==0) {
		bool isStreaming = obs_frontend_streaming_active();
		if (!isStreaming) {
			return -1;
		}
	}
	auto stream_output = obs_frontend_get_streaming_output();
	if (!stream_output) {
		return -1;
	}
	auto framerate = this->getObsFrameRate();
	if (framerate == 0) {
		obs_output_release(stream_output);
		return -1;
	}

	auto stream_frame_count = obs_output_get_total_frames(stream_output);
	auto last_stream_frame_count = stream_frame_count;
	auto stream_elapsed_time_sec = stream_frame_count / framerate;
	obs_output_release(stream_output);
	return stream_elapsed_time_sec;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrTimestamper::writeInitialCommentsToTimestampFile() {
	if (!timeStampFileIsOpen) {
		return;
	}

	std::string timeString = getCurrentDateTimeAsNiceString();
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
	if (secsOffsetIntoRecording > 0) {
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
