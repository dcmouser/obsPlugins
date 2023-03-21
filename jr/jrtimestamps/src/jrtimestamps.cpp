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

#include "jrtimestamps.hpp"
#include "jrtimestamps_options.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"
#include "pluginInfo.hpp"

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
	blog(LOG_WARNING, "deleting.");

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
	if (id == hotkeyId_triggerTimestamp) {
		writeTimestamp("Hotkey triggered", true);
	}
}
//---------------------------------------------------------------------------












































//---------------------------------------------------------------------------
void jrTimestamper::resetTimestampOrigin(std::string label, bool flagWithWallClockTime) {
	sessionCounter = 0;
	timestampOrigin = clock();
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
	std::string filePath = calcTimestampFilePath(prefix, timestampFileSuffix);
	//blog(LOG_WARNING, "Timestamp file path: %s.", filePath.c_str());
	// open file
	timestampFilep = os_fopen(filePath.c_str(), "w");
	//
	timeStampFileIsOpen = (timestampFilep!=NULL);
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
	fclose(timestampFilep);
	timestampFilep = NULL;
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
	std::string timeString = getCurrentTimesOffsetAsString();
	std::string line = timeString + " - " + label + parenExtra + "\n";

	// ATTN: TODO write line
	fputs(line.c_str(), timestampFilep);
	if (option_FlushFileEveryLine) {
		fflush(timestampFilep);
	}
	//blog(LOG_WARNING, "Logging timestamp: %s.", line.c_str());

	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
std::string jrTimestamper::getCurrentTimesOffsetAsString() {
	clock_t nowTime = clock();
	clock_t offset = nowTime - timestampOrigin;
	unsigned long secs = offset / CLOCKS_PER_SEC;

	if (optionTimestampAdjustSecs > 0 ||  (optionTimestampAdjustSecs<0 && secs > (unsigned long) (- 1 * optionTimestampAdjustSecs))) {
		secs += optionTimestampAdjustSecs;
	}

	return calcSecsAsNiceTimeString(secs, optionPadLeadingZeros);
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
	fputs(line.c_str(), timestampFilep);
	if (option_FlushFileEveryLine) {
		fflush(timestampFilep);
	}
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------

void jrTimestamper::loadStuff(obs_data_t *settings) {
	obs_data_array_t *hotkeys = obs_data_get_array(settings, "jrTimestamps.timestampTrigger");
	if (hotkeyId_triggerTimestamp!=-1 && obs_data_array_count(hotkeys)) {
		obs_hotkey_load(hotkeyId_triggerTimestamp, hotkeys);
	}
	obs_data_array_release(hotkeys);
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
	obs_data_array_t *hotkeys = obs_hotkey_save(hotkeyId_triggerTimestamp);
	obs_data_set_array(settings, "jrTimestamps.timestampTrigger", hotkeys);
	obs_data_array_release(hotkeys);
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
	if (hotkeyId_triggerTimestamp==-1) hotkeyId_triggerTimestamp = obs_hotkey_register_frontend("jrTimestamps.timestampTrigger", "jrTimestamps - Record a timestamp entry in file", ObsHotkeyCallback, this);
}

void jrTimestamper::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
	//
	if (hotkeyId_triggerTimestamp != -1) {
		obs_hotkey_unregister(hotkeyId_triggerTimestamp);
	}
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
