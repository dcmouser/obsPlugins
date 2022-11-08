//---------------------------------------------------------------------------
#include <cstdio>
#include "jrPlugin.h"
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
void JrPlugin::gotoCurrentMarkerlessCoordinates() {
	markerlessCycleListAdvance(0);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrPlugin::viewCycleAdvance(int delta) {
	if (opt_enableMarkerlessUse) {
		markerlessCycleListAdvance(delta);
	}
	else {
		// if markerless use is disabled, we cycle sources
		viewingSourceManuallyAdvance(delta);
	}
}

void JrPlugin::markerlessCycleListAdvance(int delta) {
	opt_markerlessCycleIndex += delta;
	opt_markerlessCycleIndex = opt_markerlessCycleIndex % stracker.markerlessManager.getEntryCount();
	if (opt_markerlessCycleIndex < 0) {
		opt_markerlessCycleIndex += stracker.markerlessManager.getEntryCount();
	}

	mydebug("markerlessCycleListAdvanceDelta: %d mod %d.", opt_markerlessCycleIndex,stracker.markerlessManager.getEntryCount());

	// switch what we see if we are markerless -- the true option here says switch sources to the new source even if normal autohunt is off
	if (true) {
		// travel to markerless locations EVEN if we have a target, we can rehunt to proper location after
		// so if they manually change they will see the effect even if we are zoomed in
		stracker.travelToMarkerlessInitiate(true);
		stracker.goDirectlyToMakersAndDelayHunting();
	}
	else {
		stracker.reTravelToMarkerlessIfMarkerless(true);
	}
}


void JrPlugin::viewingSourceManuallyAdvance(int delta) {
	// cycle through what source we are viewing -- useful to put on a hotkey to test multiple source cycling stuff
	// NOTE that if we are not in non-autoswitch source mode, we may jump out of this new source almost immediately -- it should only really be called when autoswitch sources is off
	stracker.sourceIndexViewing += delta;
	stracker.sourceIndexViewing = stracker.sourceIndexViewing % stracker.sourceCount;
	if (stracker.sourceIndexViewing < 0) {
		stracker.sourceIndexViewing += stracker.sourceCount;
	}

	//reTravelToMarkerlessIfMarkerless(false);
	stracker.travelToMarkerlessInitiate(true);
	stracker.goDirectlyToMakersAndDelayHunting();
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void  JrPlugin::initiateFade(int startingSourceIndex, int endingSourceIndex) {
	if (opt_fadeDuration < 0.0001f || opt_fadeMode==0) {
		// fade disabled
		return;
	}

	fadeStartingSourceIndex = startingSourceIndex;
	fadeEndingSourceIndex = endingSourceIndex;
	fadePosition = 0.0f;
	fadeStartTime = clock();
	fadeDuration = (clock_t) (opt_fadeDuration * (float)CLOCKS_PER_SEC);
	fadeEndTime = fadeStartTime + fadeDuration;
	// set viewing to the target
	stracker.sourceIndexViewing = endingSourceIndex;
}

void  JrPlugin::cancelFade() {
	fadeStartingSourceIndex = -1;
	fadeEndingSourceIndex = -1;
	fadePosition = 1.0f;
}

bool JrPlugin::updateFadePosition() {
	if (fadeStartingSourceIndex == -1) {
		return false;
	}
	clock_t curtime = clock();
	if (curtime > fadeEndTime) {
		cancelFade();
		return false;
	}
	fadePosition = (float)(fadeEndTime-curtime) / (float)fadeDuration;
	return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::cancelOneShot() {
	oneShotEngaged = false;
}


void JrPlugin::initiateOneShot() {
	// turn off auto updating
	opt_enableAutoUpdate = false;
	// turn off ignore if it is on?
	opt_ignoreMarkers = false;
	saveVolatileSettings();
	// abort any hunting
	stracker.abortHunting();
	// start oneshot
	oneShotEngaged = true;
	oneShotStage = 0;
}


void JrPlugin::updateOneShotStatus(bool isStill) {
	// this is also where oneshot gets turned off
	//mydebug("In updateOneSHot status %d and stage: %d.", (int) oneShotEngaged, oneShotStage);
	if (!isOneShotEngaged()) {
		return;
	}
	//mydebug("In updateOneSHot stage: %d.", oneShotStage);
	if (oneShotStage == 0 || !isStill) {
		// switch into stage 1 which allows hunting for a short period, and reset clock
		// we also do this if we detect some movement before we get to the end
		oneShotEndTime = (clock_t)(clock() + ((float)CLOCKS_PER_SEC/ 1000.0f) * (float)DEF_MsSecsToHuntAfterOneShotSettles);
		oneShotStage = 1;
	} else {
		// beyond stage 0 we check time and turn off
		if (clock() > oneShotEndTime) {
			// turn off tracking for oneshot
			cancelOneShot();
		}
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrPlugin::goToInitialView() {
	// go to initial markerless view
	stracker.travelToMarkerlessDefault();
}
//---------------------------------------------------------------------------


