#include "jrPlugin.h"
#include "jrfuncs.h"
#include "jrRegionDetector.h"





//---------------------------------------------------------------------------
bool JrPlugin::calibrateMarkerSizes() {
	// we are going to be setting source 0 settings

	// first we need to make sure 2 good markers were found
	TrackedSource* tsp = stracker.getTrackedSourceByIndex(0);
	if (!tsp || !tsp->areMarkersBothVisibleNeitherOccluded()) {
		return false;
	}

	// ok let's get avg size of region
	JrRegionSummary* region1p = &tsp->lastGoodMarkerRegion1;
	JrRegionSummary* region2p = &tsp->lastGoodMarkerRegion2;
	//
	float area1 = (float)region1p->area;
	float area2 = (float)region2p->area;
	float area = (float)sqrt(max(area1, area2));

	// ok set size min and max to 75% and 1.25% of found marker size
	opt_rmSizeMin = (int)(area * 0.50f / tsp->stageDistMultOpt);
	opt_rmSizeMax = (int)(area * 4.0f / tsp->stageDistMultOpt);
	opt_rmTDensityMin = 0.45f;
	opt_rmTAspectMin = 0.45f;
	opt_rmTooCloseDist = (int)((float)opt_rmSizeMax * 2.0f);

	// reset zoomscale to 1
	stracker.sourceZoomScale[0] = 1.0f;

	// ok all done now SAVE values to options
	saveVolatileMarkerZoomScaleSettings(false);

	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#define JrRgb(a,r,g,b) ((uint32_t)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))

bool JrPlugin::calibrateMarkerChroma() {
	// ok this one is tricky.. what we would like to do is scan the pixels on the stage memory in regions under the markers
	// and then compute the AVERAGE of the pixel colors already found to be within chroma
	// this will be our CHROMA COLOR
	// then we have the similarity and smoothness parameters, which we could try to set based on the variance of color, or just let user set it

	if (DefDebugDisableChromaCalibrate) {
		return false;
	}

	// first we need to make sure 2 good markers were found
	TrackedSource* tsp = stracker.getTrackedSourceByIndex(0);
	if (!tsp || !tsp->areMarkersBothVisibleNeitherOccluded()) {
		return false;
	}

	// this could cause crashes?
	if (false) {
		obs_enter_graphics();
		tsp->touchRefreshDuringRenderCycle();
		obs_leave_graphics();
	}

	// ok let's get avg size of region
	JrRegionDetector* rdp = &tsp->regionDetector;
	JrRegionSummary* region1p = &tsp->lastGoodMarkerRegion1;
	JrRegionSummary* region2p = &tsp->lastGoodMarkerRegion2;

	// set chroma color
	unsigned long colorSumR = 0;
	unsigned long colorSumG = 0;
	unsigned long colorSumB = 0;
	unsigned long foregroundPixelsFount = 0;

	rdp->calibrateScanRegion(region1p, colorSumR, colorSumG, colorSumB, foregroundPixelsFount, tsp->stagedDataLineSize, tsp->stagedData);
	rdp->calibrateScanRegion(region2p, colorSumR, colorSumG, colorSumB, foregroundPixelsFount, tsp->stagedDataLineSize, tsp->stagedData);
	//
	//mydebug("Chroma calibration: %lu %lu %lu (%lu).", colorSumR, colorSumG, colorSumB, pixelCount);
	if (foregroundPixelsFount > 0) {
		double colorR = (double)colorSumR / (double)foregroundPixelsFount;
		double colorG = (double)colorSumG / (double)foregroundPixelsFount;
		double colorB = (double)colorSumB / (double)foregroundPixelsFount;
		//opt_key_color = JrRgb(0xff, (unsigned char)colorR, (unsigned char)colorG, (unsigned char)colorB);
		opt_key_color1 = JrRgb(0xff, (unsigned char)colorB, (unsigned char)colorG, (unsigned char)colorR);
		//opt_key_color2 = JrRgb(0xff, (unsigned char)colorB, (unsigned char)colorG, (unsigned char)colorR);
	}

	// ok all done now SAVE values to options
	saveVolatileMarkerZoomScaleSettings(true);

	return true;
}

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool JrPlugin::calibrateSourceZoomScales() {
	// 1. go through all sources and tell them to find markers
	// 2. make sure source 0 found markers, and measure baseline marker region size
	// 3. then for each other source calculate zoom scale as relative distance from its marker to baseline
	TrackedSource* tsp;

	int sourceCount = stracker.getSourceCount();
	if (sourceCount < 2) {
		return false;
	}

	// ATTN: ideally we'd like to automatically trigger a rescan of sources by doing what OnPropertyButtonClickTrackOnAllSources() does
	// and then waiting at least one render cycle before continue.. this is the hard part, letting some render cycles pass
	// so for now we require user to manually press button first before triggering this option


	// ok baseline markers, make sure both are visible.
	tsp = stracker.getTrackedSourceByIndex(0);
	if (!tsp || !tsp->areMarkersBothVisibleNeitherOccluded()) {
		return false;
	}

	// ok we have our baseline
	float baselineDist = (float)jrPointDist(tsp->markerx1, tsp->markery1, tsp->markerx2, tsp->markery2);
	if (baselineDist <= 1.0f) {
		return false;
	}

	// this should be one, but we'll allow it to be whatever user wants
	float baselineZoomScale = stracker.sourceZoomScale[0];

	// now adjust others do the otherss
	for (int i = 1; i < sourceCount; ++i) {
		tsp = stracker.getTrackedSourceByIndex(i);
		if (!tsp->areMarkersBothVisibleNeitherOccluded()) {
			continue;
		}
		float mdist = (float)jrPointDist(tsp->markerx1, tsp->markery1, tsp->markerx2, tsp->markery2);
		if (mdist <= 1) {
			continue;
		}
		// ok now set its scale based on change in baseline dist
		if (true) {
			stracker.sourceZoomScale[i] = baselineZoomScale * (mdist / baselineDist);
		}
		else {
			stracker.sourceZoomScale[i] = 1.0f;
		}
	}

	// ok all done now SAVE values to options
	saveVolatileMarkerZoomScaleSettings(false);
	return true;
}
//---------------------------------------------------------------------------












