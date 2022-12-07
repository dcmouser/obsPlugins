//---------------------------------------------------------------------------
#include "jrSourceTracker.h"
#include "jrPlugin.h"
#include "obsHelpers.h"
#include "jrfuncs.h"
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
float TrackedSource::calcMinDistanceBetweenRegions(JrRegionSummary* region1, JrRegionSummary* region2) {
	// we want some kind of heuristic distance between two regions
	// could use midpoints, or min distance between any two corners
	// inner distance (closet points distance)
	// lets just do midpoints
	int r1midx = (region1->x1 + region1->x2) / 2;
	int r2midx = (region2->x1 + region2->x2) / 2;
	int r1midy = (region1->y1 + region1->y2) / 2;
	int r2midy = (region2->y1 + region2->y2) / 2;
	float dist = (float)jrPointDist(r1midx, r1midy, r2midx, r2midy);
	return dist;
}




bool TrackedSource::isRegionSimilarBounds(JrRegionSummary* regiona, JrRegionSummary* regionb) {
	// to decide if region is in same location as before
	JrPlugin* plugin = getPluginp();
	// for distance treatd as similar we reuse the existing parameter for box move distance(?)
	float distInStageSpace = calcWorstRegionPointDist(regiona, regionb);
	if (distInStageSpace <= plugin->opt_zcReactionDistance * stageDistMultOpt) {
		// close enough
		//mydebug("ATTN: isRegionSimilarBounds GOOD with distance = %f < %f.", distInStageSpace, plugin->opt_zcReactionDistance * stageDistMultOpt);
		return true;
	}
	//mydebug("ATTN: isRegionSimilarBounds BAD with distance = %f > %f.", distInStageSpace, plugin->opt_zcReactionDistance * stageDistMultOpt);
	return false;
}


float TrackedSource::calcWorstRegionPointDist(JrRegionSummary* regiona, JrRegionSummary* regionb) {
	// approximate distance measure
	float dist;
	if (true) {
		int p1dist = (int)jrPointDist(regiona->x1, regiona->y1, regionb->x1, regionb->y1);
		int p2dist = (int)jrPointDist(regiona->x1, regiona->y2, regionb->x1, regionb->y2);
		int p3dist = (int)jrPointDist(regiona->x2, regiona->y2, regionb->x2, regionb->y2);
		int p4dist = (int)jrPointDist(regiona->x2, regiona->y1, regionb->x2, regionb->y1);
		dist = (float)jrmax4(p1dist, p2dist, p3dist, p4dist);
	}
	else {
		int p1dist = (int)max(fabs(regiona->x1 - regionb->x1), fabs(regiona->y1 - regionb->y1));
		int p2dist = (int)max(fabs(regiona->x2 - regionb->x2), fabs(regiona->y2 - regionb->y2));
		dist = (float)(p1dist+p2dist);
	}
	return dist;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void TrackedSource::findTrackingMarkerRegionInSource(obs_source_t* source,  bool shouldUpdateTrackingBox, bool isHunting, bool debugPreviewOnly) {
	JrPlugin* plugin = getPluginp();

	// experiment
	//obs_source_inc_showing(source);

	// part 1
	//mydebug("findTrackingMarkerRegionInSource - part 1 [%d ishunting = %d]  [source showing = %d]",index,(int)isHunting,(int)obs_source_showing(source));
	// Render to intermediate target texrender instead of output of plugin (screen)
	doRenderWorkFromEffectToStageTexRender(source);

	if (DefDilateImplementationEffect) {
		// do dilation after chroma
		// from staging stagingTexrender to stagingTexrender
		doRender_Dilate_Effect_OnStagingTexrender(plugin->opt_dilateGreenSteps, plugin->opt_dilateRedSteps);
	}


	// part 2
	// ok now the output is in texrender texture where we can map it and copy it to private user memory
	// this should always return true;
	//mydebug("findTrackingMarkerRegionInSource - part 2");	
	bool bretv = doRenderWorkFromStageToInternalMemory();
	if (bretv) {
		// ATTN: IMPORTANT -- whatever you do here you may have to do in the kludge function touchRefreshDuringRenderCycle()
		// preprocessing does some dilation
		doRenderToInternalMemoryPostProcessing();
	}

	// part 2b
	if (bretv) {
		// part 3
		// update autotracking by doing machine vision on internal memory copy of effectChromaKey output
		if (shouldUpdateTrackingBox) {
			//mydebug("Doing part 3 stracker.analyzeSceneAndFindTrackingBox.");
			//mydebug("findTrackingMarkerRegionInSource - part 3");
			analyzeSceneAndFindTrackingBox(stagedData, stagedDataLineSize, isHunting, debugPreviewOnly);
		}
	}

	// experiment
	//obs_source_dec_showing(source);
	
	//mydebug("In findTrackingMarkerRegionInSource with index %d and isHunting = %d and shouldupdateTracking = *** %d ***  regions are (%d,%d).", index, (bool)isHunting, (bool)shouldUpdateTrackingBox, regionDetector.foundRegionsValid, regionDetector.foundRegions);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void TrackedSource::markRegionsQuality() {
	JrRegionDetector* rd = &regionDetector;
	JrRegionSummary* region;

	int foundValidCount = 0;
	for (int i = 0; i < rd->foundRegions; ++i) {
		region = &rd->regions[i];
		region->valid = calcIsValidmarkerRegion(region);
		if (region->valid) {
			++foundValidCount;
		}
	}
	rd->foundRegionsValid = foundValidCount;
}


bool TrackedSource::calcIsValidmarkerRegion(JrRegionSummary* region) {
	// valid region based on plugin options
	bool flagCountOnlyInterirorRedWhite = true;

	JrPlugin* plugin = getPluginp();

	if (region->pixelCountAll == 0) {
		return false;
	}

	if (region->density < plugin->opt_rmTDensityMin) {
		return false;
	}
	if (region->aspect < plugin->opt_rmTAspectMin) {
		return false;
	}

	// ok min and max sizes are based on AREA
	double areaAdjusted = sqrt((double)region->area ) / (double)stageDistMultOpt;
	if (areaAdjusted < plugin->opt_rmSizeMin || (plugin->opt_rmSizeMax>0 && areaAdjusted > plugin->opt_rmSizeMax) ) {
		return false;
	}

	// dual color percentages
	if (plugin->opt_markerMultiColorMode > 1 && region->pixelCountAll>0) {

		float percentageColor2;
		if (flagCountOnlyInterirorRedWhite) {
			percentageColor2 = (float)(region->pixelCountInteriorPerEnumColor[DefRdPixelEnumColor2] + region->pixelCountInteriorPerEnumColor[DefRdPixelEnumColor3]) / (float)region->pixelCountAll;
		} else {
			percentageColor2 = (float)(region->pixelCountPerEnumColor[DefRdPixelEnumColor2] + region->pixelCountPerEnumColor[DefRdPixelEnumColor3]) / (float)region->pixelCountAll;
		}
		if (percentageColor2 < plugin->opt_rmMinColor2Percent || percentageColor2 > plugin->opt_rmMaxColor2Percent) {
			//mydebug("Rejecting marker %d due to dual percentages of %f and %d / %d (%d) / %d with total %d (int=%d bord=%d).", region->label, percentageColor2, region->pixelCountPerEnumColor[DefRdPixelEnumColor1], region->pixelCountPerEnumColor[DefRdPixelEnumColor2], region->pixelCountInteriorPerEnumColor[DefRdPixelEnumColor2], region->pixelCountPerEnumColor[DefRdPixelEnumColor3], region->pixelCountAll, region->pixelCountInterior, region->pixelCountBorder);
			return false;
		}
	}

	// valid
	return true;
}



// ATTN: 9/14/22 we have a crash caused by excessive regions inside here (though the crash doesnt happen here)
void TrackedSource::analyzeSceneAndFindTrackingBox( uint8_t *data, uint32_t dlinesize, bool isHunting, bool debugPreviewOnly) {

	//mydebug("In analyzeSceneAndFindTrackingBox.");

	// ATTN: the crash happens here (unsurprisingly)

	// step 1 - build labeling matrix
	// this does the possibly heavy duty computation of Connected Components Contour Tracing, etc.
	analyzeSceneFindComponentMarkers(data,dlinesize);


	// step 2 identify the new candidate tracking box in the view
	// this may find nothing reliable -- that's ok
	//mydebug("in analyzeSceneAndFindTrackingBox in index %d.", index);
	findNewCandidateTrackingBox(debugPreviewOnly);

	if (areMarkersBothVisibleNeitherOccluded()) {
		//mydebug("found good marker Region in index %d.",index);
		if (!debugPreviewOnly && isHunting) {
			// we were hunting and we found a good new source to switch to
			//mydebug("!!!!!!!!!!!!!!!! in analyzeSceneAndFindTrackingBox hit one index %d.", index);
			sourceTrackerp->foundGoodHuntedSourceIndex(index);
		}
		else {
			//mydebug("in analyzeScene but isHunting false so doing nothing.");
		}
	} else {
		//mydebug("Did not find markers in source with index = %d and huntingIndex = %d, validRegions = %d of %d.", index, huntingIndex, regionDetector.foundRegionsValid, regionDetector.foundRegions);
		//mydebug("Did not find markers in source with index = %d, validRegions = %d of %d.", index, regionDetector.foundRegionsValid, regionDetector.foundRegions);
	}
}



// ATTN: 9/14/22 - crash happens in here
void TrackedSource::analyzeSceneFindComponentMarkers(uint8_t *data, uint32_t dlinesize) {

	JrRegionDetector* rd = &regionDetector;

	// step 1 build initial labels
	if (!data) {
		//mydebug("Error data buffer for analyzeSceneFindComponentMarkers is null.");
		rd->clearComputations();
		return;
	}

	int foregroundPixelCount = rd->fillFromStagingMemory((uint32_t*) data, dlinesize);

	// step 2 - connected component labelsing
	int regionCountFound = rd->doConnectedComponentLabeling();

	// debug
	//mydebug("JrObs CCL regioncount = %d (%d foregroundpixels) regionsize = [%d,%d].", regionCountFound, foregroundPixelCount, rd->width, rd->height);
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void TrackedSource::doRenderToInternalMemoryPostProcessing() {
	// dilate
	if (plugin->opt_dilateGreenSteps > 0 || plugin->opt_dilateRedSteps > 0) {
		regionDetector.doRenderToInternalMemoryPostProcessing_Dilate((uint32_t*)stagedData, stagedDataLineSize, plugin->opt_dilateGreenSteps, plugin->opt_dilateRedSteps);
	}

	if (plugin->opt_chromaDualColorGapFill > 0 && (plugin->opt_markerMultiColorMode == 2 || plugin->opt_markerMultiColorMode == 5)) {
		// add white pixels that span orthonogonal gaps between green and red to make them look like a single solid region to region extractor
		regionDetector.doRenderToInternalMemoryPostProcessing_DualColorGapFill((uint32_t*)stagedData, stagedDataLineSize, plugin->opt_chromaDualColorGapFill);
	}
}
//---------------------------------------------------------------------------
