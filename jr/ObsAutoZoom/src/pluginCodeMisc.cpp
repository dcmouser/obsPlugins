//---------------------------------------------------------------------------
#include <cstdio>
#include "jrPlugin.h"
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
	//mydebug("ATTN:oneshot - canceling.");
	oneShotEngaged = false;
	oneShotFoundTarget = false;
}


void JrPlugin::initiateOneShot() {
	// turn off auto updating
	opt_autoTrack = false;
	saveVolatileSettings();
	// abort any hunting
	stracker.abortHunting();
	// start oneshot
	oneShotEngaged = true;
	oneShotStage = 0;
	oneShotFoundTarget = false;
	oneShotDidAtLeastOneTrack = false;
	//mydebug("ATTN:oneshot - initiating.");
}


void JrPlugin::updateOneShotStatus(bool isStill, bool goodMarkersFound) {
	// this is also where oneshot gets turned off
	//mydebug("In updateOneSHot status %d and stage: %d.", (int) oneShotEngaged, oneShotStage);
	if (!isOneShotEngaged()) {
		return;
	}

	if (!oneShotDidAtLeastOneTrack) {
		// ATTN: this new code ensures that we dont change these findings on a render tick() without a tracking tick
		return;
	}

	if (goodMarkersFound && !oneShotFoundTarget) {
		setOneShotFoundValidTarget(true);
		//mydebug("ATTN:oneshot - Setting oneshot found valid target.");
	}

	//mydebug("In updateOneSHot stage: %d.", oneShotStage);
	if (oneShotStage == 0 || !isStill) {
		// switch into stage 1 which allows hunting for a short period, and reset clock
		// we also do this if we detect some movement before we get to the end
		//mydebug("ATTN:oneshot entering stage 1 not still = %d.",(int)isStill);
		oneShotEndTime = (clock_t)(clock() + ((float)CLOCKS_PER_SEC/ 1000.0f) * (float)DEF_MsSecsToHuntAfterOneShotSettles);
		oneShotStage = 1;
	} else {
		// beyond stage 0 we check time and turn off
		if (clock() > oneShotEndTime) {
			// turn off tracking for oneshot
			//mydebug("ATTN:oneshot expires.");
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
























































//---------------------------------------------------------------------------
void JrPlugin::gotoCurrentMarkerlessCoordinates() {
	if (true) {
		stracker.travelToMarkerlessInitiate(true);
		stracker.goDirectlyToMakersAndDelayHunting();
	}
	else {
		// old code
		markerlessCycleListAdvance(0);
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::performViewCycleAdvance(int delta) {
	if (getMarkerlessModeIsManualZoom()) {
		doManualZoomInOutByPercent(opt_manualZoomStepSize * delta);
	} else if (getMarkerlessModeIsPresets()) {
		markerlessCycleListAdvance(delta);
	} else {
		// if markerless use is disabled, we cycle sources
		viewingSourceManuallyAdvance(delta);
	}

	// ATTN: test -- memory leak happens in this saveVolatileSettings call?

	// save current view now
	saveVolatileSettings();

	// switch what we see if we are markerless -- the true option here says switch sources to the new source even if normal autohunt is off
	if (true) {
		// travel to markerless locations EVEN if we have a target and markers, we can rehunt to proper location after
		// so if they manually change they will see the effect even if we are zoomed in, so they can see that they tried to set markerless even while markers are present and controlling
		stracker.travelToMarkerlessInitiate(true);
		stracker.goDirectlyToMakersAndDelayHunting();
	}
	else {
		stracker.reTravelToMarkerlessIfMarkerless(true);
	}
}

void JrPlugin::markerlessCycleListAdvance(int delta) {
	opt_markerlessCycleIndex += delta;
	opt_markerlessCycleIndex = opt_markerlessCycleIndex % stracker.markerlessManager.getEntryCount();
	if (opt_markerlessCycleIndex < 0) {
		opt_markerlessCycleIndex += stracker.markerlessManager.getEntryCount();
	}
	mydebug("markerlessCycleListAdvanceDelta: %d mod %d.", opt_markerlessCycleIndex,stracker.markerlessManager.getEntryCount());
}


void JrPlugin::viewingSourceManuallyAdvance(int delta) {
	// this function is used when the markerless options are diabled and hotkey rotates through sources.. very little reason to use this mode
	// cycle through what source we are viewing -- useful to put on a hotkey to test multiple source cycling stuff
	// NOTE that if we are not in non-autoswitch source mode, we may jump out of this new source almost immediately -- it should only really be called when autoswitch sources is off
	stracker.sourceIndexViewing += delta;
	stracker.sourceIndexViewing = stracker.sourceIndexViewing % stracker.sourceCount;
	if (stracker.sourceIndexViewing < 0) {
		stracker.sourceIndexViewing += stracker.sourceCount;
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrPlugin::doManualZoomInOutByPercent(float scalePercent) {

	// tell the source tracker about our new markerless
	int sourceIndex = opt_manualZoomSourceIndex;
	float zoomLevel = opt_manualZoomSourceScale;
	int alignmentMode = opt_manualZoomAlignment;

	// ok now adjust zoom level by a certain percent
	// note that this MIGHT cause us to transition to a wider or closer source if multiple camera source, based on 	float opt_manualZoomTransitions[DefMaxSources];

	// ok adust zoom
	if (scalePercent == 0.0) {
		// nothing
	} else if (scalePercent < 0.0) {
		// attempt to go up and down symmetrically instead of more
		zoomLevel = zoomLevel / (float)(1.0 - scalePercent);
	} else if (scalePercent > 0.0) {
		zoomLevel = zoomLevel + (scalePercent * zoomLevel);
	}

	// might want this configured based on aspect ratio of display -- let user configure it?
	float minZoom = opt_manualZoomMinZoom;

	//mydebug("In doManualZoomInOutByPercent %f.", zoomLevel);
	// note that we are not allowed to go below 1.0
	if (zoomLevel < minZoom) {
		// ok so either we have to pop to a wider source or clamp here
		if (sourceIndex < 1) {
			// clamp
			zoomLevel = minZoom;
		}
		else {
			// drop to wider source
			--sourceIndex;
			// and now init the zoom level
			zoomLevel = opt_manualZoomTransitions[sourceIndex];
			if (zoomLevel <= 0) {
				zoomLevel = 2;
			}
		}
	} else if (scalePercent >= 0.0 && sourceIndex < stracker.getSourceCount()-1) {
		// when zooming in we see if we need to switch to closer camera
		float zoomLimit = opt_manualZoomTransitions[sourceIndex];
		if (zoomLimit <= 0) {
			zoomLimit = 2.0;
		}
		if (zoomLevel > zoomLimit) {
			// move to closer source
			++sourceIndex;
			zoomLevel = 1.0;
		}
	}

	// save new values
	opt_manualZoomSourceIndex = sourceIndex;
	opt_manualZoomSourceScale = zoomLevel;

	// now update store this look
	stracker.updateManualZoomEntry(sourceIndex, zoomLevel, alignmentMode);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrPlugin::gotoLastGoodMarkerLocation() {
	// turn off auto tracking
	opt_autoTrack = false;
	// now go
	stracker.gotoLastGoodMarkerLocation();
}
//---------------------------------------------------------------------------








