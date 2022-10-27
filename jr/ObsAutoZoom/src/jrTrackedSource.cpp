//---------------------------------------------------------------------------
#include "jrTrackedSource.h"
#include "jrPlugin.h"
#include "obsHelpers.h"
#include "jrfuncs.h"

//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void TrackedSource::init(SourceTracker* st, int indexin) {
	sourceTrackerp = st;
	plugin = st->getPluginp();
	src_ref = NULL;
	//
	index = indexin;
	// this is for filter use
	externallyManaged = false;
	externallySetSourcePointer = NULL;
	//
	stageHoldingSpaceTexrender = NULL;
	sourceHoldingSpaceTexrender = NULL;
	stagingTexrender = NULL;
	stagingSurface = NULL;
	stageDrawingTexture = NULL;
	stagedData = NULL;
	//
	stageShrinkX = 	stageShrinkY = 0;
	stageArea = 0;
	stagedDataLineSize = 0;
	//
	stageDistMultOpt = 1.0f;
	//stageAreaMultOpt = 1.0f;
	sourceDistMult = 1.0f;
	sourceWidth = 0;
	sourceHeight = 0;
	//
	forceReallocate = false;
	needsRecheckForSize = false;

	// init region detector
	regionDetector.rdInit();

	// allocate texrender spaces - 9/7/22 leaking memory
	allocateGraphicsData();

	lastGoodMarkerRegion1.init();
	lastGoodMarkerRegion2.init();

	clear();
}


void TrackedSource::prepareForUseAndSetName(const char* name) {
	freeAndClearForReuse();
	allocateGraphicsData();
	strcpy(src_name, name);
}

void TrackedSource::clear() {
	freeWeakSource();

	//
	strcpy(src_name, "");
	sourceWidth = 0;
	sourceHeight = 0;
	//
	markerlessStreakCounter = markerlessStreakCycleCounter = settledLookingStreakCounter = 0;
	targetStableCount = 0; changeTargetMomentumCounter = 0;
	//delayMoveUntilHuntFinishes = false;
	//
	needsRecheckForSize = true;
	//
	clearAllBoxReadies();
}


void TrackedSource::freeForFinalExit() {
	// free the weak source reference
	freeWeakSource(); 
	// free memory use
	freeGraphicMemoryUse();
	// clear internal data
	clear();
	// tell region detector to free up
	regionDetector.rdFree();
}


void TrackedSource::freeAndClearForReuse() {
	// free the weak source reference
	freeWeakSource();

	// free memory use
	// THIS CAUSES PROBLEMS -- BUT WHY?????
	//freeGraphicMemoryUse();

	// clear internal data
	clear();
}


void TrackedSource::freeWeakSource() {
	if (src_ref != NULL) {
		obs_weak_source_release(src_ref);
		src_ref = NULL;
	}
}

void TrackedSource::freeFullSource(obs_source_t* src) {
	if (src != NULL) {
		obs_source_release(src);
	}
}



void TrackedSource::freeGraphicMemoryUse() {
	obs_enter_graphics();
	freeBeforeReallocateFilterTextures();
	freeBeforeReallocateTexRender();
	obs_leave_graphics();

	freeBeforeReallocateNonGraphicData();
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
bool TrackedSource::allocateGraphicsData() {
	// ATTN: must this be done in graphics context?
	freeBeforeReallocateTexRender();

	// create texrender
	stageHoldingSpaceTexrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	sourceHoldingSpaceTexrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	stagingTexrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	// success
	return true;
}


void TrackedSource::freeBeforeReallocateTexRender() {
	if (stageHoldingSpaceTexrender) {
		gs_texrender_destroy(stageHoldingSpaceTexrender);
		stageHoldingSpaceTexrender = NULL;
	}
	if (sourceHoldingSpaceTexrender) {
		gs_texrender_destroy(sourceHoldingSpaceTexrender);
		sourceHoldingSpaceTexrender = NULL;
	}	
	if (stagingTexrender) {
		gs_texrender_destroy(stagingTexrender);
		stagingTexrender = NULL;
	}
}

void TrackedSource::freeBeforeReallocateFilterTextures() {
	if (stagingSurface) {
		gs_stagesurface_destroy(stagingSurface);
		stagingSurface = NULL;
	}
	if (stageDrawingTexture) {
		gs_texture_destroy(stageDrawingTexture);
		stageDrawingTexture = NULL;
	}
}

void TrackedSource::freeBeforeReallocateNonGraphicData() {
	// this may have to be done LAST?
	if (stagedData) {
		bfree(stagedData);
		stagedData = NULL;
	}
}
//---------------------------------------------------------------------------































































//---------------------------------------------------------------------------
void TrackedSource::clearAllBoxReadies() {
	markerBoxReady = false;
	lookingBoxReady = false;
	stickyBoxReady = false;
	lastTargetBoxReady = false;
	markerBoxIsOccluded = false;
	//
	lookingx1 = lookingy1 = lookingx2 = lookingy2 = 0;
	markerx1 = markery1 = markerx2 = markery2 = 0;
	stickygoalx1 = stickygoaly1 = stickygoalx2 = stickygoaly2 = 0;
	//
	changeTargetMomentumCounter = 0;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void TrackedSource::clearFoundTrackingBox() {
	markerx1 = markery1 = markerx2 = markery2 = -1;
	markerBoxReady = false;
	markerBoxIsOccluded = false;
}


void TrackedSource::setStickyTargetToMarkers() {
	stickygoalx1 = (float)markerx1;
	stickygoaly1 = (float)markery1;
	stickygoalx2 = (float)markerx2;
	stickygoaly2 = (float)markery2;
	lasttargetx1 = markerx1;
	lasttargety1 = markery1;
	lasttargetx2 = markerx2;
	lasttargety2 = markery2;

	//
	changeTargetMomentumCounter = 0;
	markerlessStreakCounter = 0;
	markerlessStreakCycleCounter = 0;
	settledLookingStreakCounter = 0;
	//
	markerBoxReady = true;
	lookingBoxReady = true;
	stickyBoxReady = true;
	lastTargetBoxReady = true;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool TrackedSource::isOneValidRegionAtUnchangedLocation() {
	// IMPORTANT; you have to 
	// return true if we found one region at same location as prior tracking box
	JrRegionDetector* rd = &regionDetector;

	if (!stickyBoxReady) {
		return false;
	}
	if (rd->foundRegionsValid != 1) {
		return false;
	}

	JrRegionSummary* region;
	for (int i = 0; i < rd->foundRegions; ++i) {
		region =&rd->regions[i];
		if (region->valid) {
			// found the valid region
			// is one of the corners of the region possible at stickygoalx1 or stickygoalx2?
			if (isRegionSimilarBounds(region, &lastGoodMarkerRegion1)) {
				occludedCornerIndex = 2;
				return true;
			}
			if (isRegionSimilarBounds(region, &lastGoodMarkerRegion2)) {
				occludedCornerIndex = 1;
				return true;
			}
			break;
		}
	}
	return false;
}

//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
bool TrackedSource::findNewCandidateTrackingBox(bool debugPreviewOnly) {
	// pointers
	//mydebug("IN findNewCandidateTrackingBox 1");
	JrRegionDetector* rd = &regionDetector;
	JrPlugin* plugin = getPluginp();


	// for testing
	if (false) {
		markerx1 = 400;
		markery1 = 135;
		markerx2 = 1000;
		markery2 = 535;
		markerBoxReady = true;
		rd->foundRegions = 0;
		return true;
	}

	// helpers
	int outWidth = plugin->getPluginOutputWidth();
	int outHeight = plugin->getPluginOutputHeight();

	// reset
	clearFoundTrackingBox();

	// mark regions quality to being markers
	markRegionsQuality();

	JrRegionSummary* region;
	JrRegionSummary* region1;
	JrRegionSummary* region2;

	//mydebug("Number valid regions found: %d and total found: %d.", rd->foundRegionsValid, rd->foundRegions);

	// abort if not at least 2 regions
	if (rd->foundRegionsValid != 2) {
		// we need exactly 2 regions of validity to identify a new box
		// ATTN: TODO - we might actually be able to work with less or more IFF our current markers are CONSISTENT
		// move one region code to here

		if (DefForSingleValidRegionAssumePrevious && isOneValidRegionAtUnchangedLocation()) {
			// use last regions
			region1 = &lastGoodMarkerRegion1;
			region2 = &lastGoodMarkerRegion2;
			markerBoxIsOccluded = true;
			//mydebug("Treating one as occluded.");
			// now drop down
		} else {
			//mydebug("IN findNewCandidateTrackingBox returning 1");
			return false;
		}
	} else {
		// we found exactly 2 valid regions; get pointers to them
		int validRegionCounter = 0;
		int validRegionIndices[2];
		for (int i = 0; i < rd->foundRegions; ++i) {
			region = &rd->regions[i];
			if (region->valid) {
				validRegionIndices[validRegionCounter] = i;
				if (++validRegionCounter >= 2) {
					break;
				}
			}
		}
		// region1 and region 2
		region1=&rd->regions[validRegionIndices[0]];
		region2=&rd->regions[validRegionIndices[1]];
	}


	int posType = plugin->opt_zcMarkerPos;
	int tx1, ty1, tx2, ty2;
	int posMult = -1;
	if (posType==1) {
		// inside the markers (exclude the markers themselves)
		posMult = 1;
		if (region1->x1 <= region2->x2) {
			tx1 = region1->x2;
			tx2 = region2->x1;
		}
		else {
			tx1 = region2->x2;
			tx2 = region1->x1;
		}
		if (region1->y1 <= region2->y2) {
			ty1 = region1->y2;
			ty2 = region2->y1;
		}
		else {
			ty1 = region2->y2;
			ty2 = region1->y1;
		}
	}
	else {
		// outside the markers?
		float xszhalf1 = abs(region1->x2 - region1->x1) / 2.0f;
		float xszhalf2 = abs(region2->x2 - region2->x1) / 2.0f;
		float yszhalf1 = abs(region1->y2 - region1->y1) / 2.0f;
		float yszhalf2 = abs(region2->y2 - region2->y1) / 2.0f;
		if (region1->x1 <= region2->x2) {
			tx1 = (int)(region1->x1 + (posType==0 ? 0 : xszhalf1));
			tx2 = (int)(region2->x2 - (posType==0 ? 0 : xszhalf2));
		}
		else {
			tx1 = (int)(region2->x1 + (posType==0 ? 0 : xszhalf1));
			tx2 = (int)(region1->x2 - (posType==0 ? 0 : xszhalf2));
		}
		if (region1->y1 <= region2->y2) {
			ty1 = (int)(region1->y1 + (posType==0 ? 0 : yszhalf1));
			ty2 = (int)(region2->y2 - (posType==0 ? 0 : yszhalf2));
		}
		else {
			ty1 = (int)(region2->y1 + (posType==0 ? 0 : yszhalf1));
			ty2 = (int)(region1->y2 - (posType==0 ? 0 : yszhalf2));
		}
	}


	// check if markers are too close
	// get distance (in stage space)
	float minDistanceBetweenRegionsInStageSpace = calcMinDistanceBetweenRegions(region1, region2);
	if (minDistanceBetweenRegionsInStageSpace < plugin->computedMinDistBetweenMarkersIsTooClose * stageDistMultOpt) {
		//mydebug("Regions too close returning false.");
		return false;
	}

	// rescale from stage to original resolution
	int boxMargin = (int)((float)(plugin->opt_zcBoxMargin) * sourceDistMult);
	markerx1 = cnvStageToSourceX(tx1) + boxMargin * posMult;
	markery1 = cnvStageToSourceY(ty1) + boxMargin * posMult;
	markerx2 = cnvStageToSourceX(tx2) - boxMargin * posMult;
	markery2 = cnvStageToSourceY(ty2) - boxMargin * posMult;


	// new check if they are inline then we treat them as indicator of bottom or left
	bool horzInline = false;
	bool vertInline = false;
	//
	// new test
	// we check if midpoints are as close as avg region size, with a bonus slack allowed more distance if the aspect ratio is very skewed

	// vertically inline markers?
	float midpointDist = (float)fabs(((float)(region1->x1 + region1->x2) / 2.0f) - ((float)(region2->x1 + region2->x2) / 2.0f));
	float avgRegionSize = (float)fabs(((float)(region1->x2 - region1->x1) + (region2->x2 - region2->x1)) / 2.0f);
	//float aspectBonus = abs(min((float)(markery2 - markery1),1.0f) / min((float)(markerx2 - markerx1),1.0f));
	float distx = (float)max(abs(markerx2 - markerx1), 1.0f);
	float disty = (float)max(abs(markery2 - markery1), 1.0f);
	float aspectBonus = disty / distx;
	float distThresh = (float)(avgRegionSize * (1.0f + 5.0f * min(aspectBonus, 20.0) / 20.0f));
	//info("minpointdist = %f  Avg RegionSize = %f  aspectbonus = %f distthresh = %f (distx = %f, disty=%f)", midpointDist, avgRegionSize, aspectBonus, distThresh,distx,disty);
	if (midpointDist <= distThresh) {
		vertInline = true;
	}

	// horizontally inline markers?
	midpointDist = (float)fabs(((float)(region1->y1 + region1->y2) / 2.0f) - ((float)(region2->y1 + region2->y2) / 2.0f));
	avgRegionSize = (float)((region1->y2 - region1->y1) + (region2->y2 - region2->y1)) / 2.0f;
	distx = (float)max(abs(markerx2 - markerx1), 1.0f);
	disty = (float)max(abs(markery2 - markery1), 1.0f);
	aspectBonus = distx / disty;
	distThresh = (float)(avgRegionSize * (1.0f + 5.0f * min(aspectBonus, 20.0) / 20.0f));
	if (midpointDist <= distThresh) {
		horzInline = true;
	}


	// inline aspect ratios
	if (horzInline) {
		// so we leave the x values alone and move y2 up to match aspect
		float aspectRatio = (float)outHeight / (float)outWidth;
		float sz = ((float)((markerx2 - markerx1) + 1.0f) * aspectRatio) + 1.0f;
		markery2 = (int)((float)markery1 + sz);
		if ((unsigned int)markery2 > sourceHeight - 1) {
			int dif = markery2 - (sourceHeight - 1);
			markery2 = sourceHeight - 1;
			markery1 = (int)(markery2 - sz);
		}
	}
	else if (vertInline) {
		// so we leave the y values alone and move x2 up to match aspect
		float aspectRatio = (float)outWidth / (float)outHeight;
		float sz = ((float)((markery2 - markery1) + 1.0f) * aspectRatio) - 1.0f;
		markerx2 = (int)((float)markerx1 + sz);
		if ((unsigned int)markerx2 > sourceWidth-1) {
			int dif = markerx2 - (sourceWidth-1);
			markerx2 = sourceWidth-1;
			markerx1 = (int)(markerx2 - sz);
		}
	}

	// clean up bounds
	if (markerx1 < 0) {
		markerx1 = 0;
	}
	if (markery1 < 0) {
		markery1 = 0;
	}
	if ((unsigned int)markerx1 >= sourceWidth) {
		markerx1 = sourceWidth-1;
	}
	if ((unsigned int)markery1 >= sourceHeight) {
		markery1 = sourceHeight-1;
	}
	if (markerx2 < 0) {
		markerx2 = 0;
	}
	if (markery2 < 0) {
		markery2 = 0;
	}
	if ((unsigned int)markerx2 >= sourceWidth) {
		markerx2 = sourceWidth-1;
	}
	if ((unsigned int)markery2 >= sourceHeight) {
		markery2 = sourceHeight-1;
	}

	// remember last two good regions (unless we are already using them due to finding a single marker)
	if (&lastGoodMarkerRegion1 != region1) {
		memcpy(&lastGoodMarkerRegion1, region1, sizeof(JrRegionSummary));
	}
	if (&lastGoodMarkerRegion2 != region1) {
		memcpy(&lastGoodMarkerRegion2, region2, sizeof(JrRegionSummary));
	}

	//mydebug("Returning true from region marker find.");

	//mydebug("IN findNewCandidateTrackingBox 5");

	// tracking box now has valid data
	markerBoxReady = true;
	return markerBoxReady;
}
//---------------------------------------------------------------------------


























//---------------------------------------------------------------------------
void TrackedSource::setStageSize(int instageWidth, int instageHeight) {
	// tell region detctor about resize
	stageWidth = instageWidth;
	stageHeight = instageHeight;
	stageArea = stageWidth * stageHeight;
	regionDetector.rdResize(instageWidth, instageHeight);
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
obs_source_t* TrackedSource::borrowFullSourceFromWeakSource() {
	if (externallyManaged) {
		// we dont borrow a strong source we just returned the stored one
		//mydebug("TrackedSource %d in releaseBorrowedFullSource is externally managed.", index);
		return externallySetSourcePointer;
	}
	// request a strong source from weak from obs -- this needs to be returned!
	return obs_weak_source_get_source(src_ref);
}


void TrackedSource::releaseBorrowedFullSource(obs_source_t* sourcep) {
	if (externallyManaged) {
		// we didnt borrow a strong source we just returned the stored one, so we do nothing here
		//mydebug("TrackedSource %d in releaseBorrowedFullSource is externally managed.", index);
		return;
	}
	freeFullSource(sourcep);
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void TrackedSource::onOptionsChange() {
	needsRecheckForSize = true;
}
//---------------------------------------------------------------------------




// 
// 
// 
//---------------------------------------------------------------------------
void TrackedSource::setLocationsToMarkerLocations(bool flagInstantLookingPos) {
	lasttargetx1 = markerx1;
	lasttargety1 = markery1;
	lasttargetx2 = markerx2;
	lasttargety2 = markery2;
	lastTargetBoxReady = true;

	if (true) {
		stickygoalx1 = (float)markerx1;
		stickygoaly1 = (float)markery1;
		stickygoalx2 = (float)markerx2;
		stickygoaly2 = (float)markery2;
		stickyBoxReady = true;
	}

	if (flagInstantLookingPos) {
		lookingx1 = markerx1;
		lookingy1 = markery1;
		lookingx2 = markerx2;
		lookingy2 = markery2;
		// critical -- without this these values arent trusted on startup
		lookingBoxReady = true;
	}
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
void TrackedSource::updateAfterHuntCheck() {
	// keep track of last hunt location
	lasthuntx1 = markerx1;
	lasthunty1 = markery1;
	lasthuntx2 = markerx2;
	lasthunty2 = markery2;
}
//---------------------------------------------------------------------------












//---------------------------------------------------------------------------
void TrackedSource::touchRefreshDuringRenderCycle() {
	JrPlugin* plugin = getPluginp();

	if (DefDebugTestSkipKludgeRefreshTouchRendering) {
		return;
	}

	// lease it
	obs_source_t* osp = borrowFullSourceFromWeakSource();

	// resize check
	if (osp) {
		recheckSizeAndAdjustIfNeeded(osp);
	}

	if (osp == NULL || sourceWidth == 0 || stagedData == NULL) {
		// aborting touch routine
		// release full source
		releaseBorrowedFullSource(osp);
		//mydebug("Aborting unready source in touchRefreshDuringRenderCycle.");
		return;
	}

	// want to do something else?
	//obs_source_inc_showing(osp);

	// both of these seem necesary in order to ensure that we have fresh data reloaded for the source when we need it
	// why would both be needed??
	doRenderWorkFromEffectToStageTexRender(plugin->effectChroma, osp);
	bool bretv = doRenderWorkFromStageToInternalMemory();
	if (bretv && (plugin->opt_debugChroma || plugin->opt_debugRegions)) {
		// ATTN: IMPORTANT -- whatever you do here you may have to do in the findTrackingMarkerRegionInSource function
		// preprocessing does some dilation
		doRenderToInternalMemoryPostProcessing();
	}

	//obs_source_dec_showing(osp);

	// release it
	releaseBorrowedFullSource(osp);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TrackedSource::setExternallyManagedSource(obs_source_t* sourcep) {
	prepareForUseAndSetName("");
	externallyManaged = true;
	externallySetSourcePointer = sourcep;
}
//---------------------------------------------------------------------------















































//---------------------------------------------------------------------------
void TrackedSource::travelToTrackedSourceMarkerless(JrMarkerlessEntry *entryp, bool forceInstant, bool useFade) {
	// this is called when we hunt for markers in a different view and FAIL to find them.. so it is what now says finally that we should travel to markerless coordinates (and optionally change sources)
	// note the caller is responsible for setting sourcetracker viewindex to us
	//mydebug("In travelToMarkerless for source index #%d.", index);
	JrPlugin* plugin = getPluginp();


	// first set OUR marker locations (or to defaults)
	setMarkerCoordsFromMarklessEntry(entryp);

	// set other coords based on our now updated markers, including looking if forceInstant
	setLocationsToMarkerLocations(forceInstant);

	// independent from whether there is an instant set of our looking location, if we are on a different source we may fade
	// ok now transition fade or move?
	if (sourceTrackerp->getViewSourceIndex() == index || !plugin->opt_enableMarkerlessUse) {
		// we are already the view source, so we just need to set target locations, not switch over to this source; we will graduall move to the lasttarget locations set above
	} else {
		// ATTN: TODO there are times in our old code when we did NOT fade if pushing in -- we are missing that here
		sourceTrackerp->setViewSourceIndex(index,useFade);
		// always force instant destination positions when fading between sources
		if (useFade) {
			setLocationsToMarkerLocations(true);
		}
	}

	// and cancel any oneshot -- this makes it easier to cover up markers and trigger a one shot without it maintaining prolonged one shot tracking
	if (true) {
		plugin->cancelOneShot();
	}
}






void TrackedSource::setMarkerCoordsToBoundaries() {
	setMarkerCoords(0, 0, sourceWidth, sourceHeight);
}



void TrackedSource::setMarkerCoordsFromMarklessEntry(JrMarkerlessEntry* entryp) {
	JrPlugin* plugin = getPluginp();
	// sanity check
	if (entryp == NULL || sourceWidth <= 0 || sourceHeight <= 0 || !plugin->opt_enableMarkerlessUse) {
		setMarkerCoordsToBoundaries();
		return;
	}

	bool fitSmallDimension = false;

	int outWidth = plugin->getPluginOutputWidth();
	int outHeight = plugin->getPluginOutputHeight();
	// now compute markerless coords for this source
	float outAspect = (float)outWidth / (float)outHeight;
	float sourceAspect = (float)sourceWidth / (float)sourceHeight;

	float widthReduction = 1.0;// outWidth / sourceWidth;
	float heightReduction = 1.0; // outHeight / sourceHeight;

	if (entryp->zoomLevel <= 0.0f) {
		// show entire source scaled to fit longest dimension (normal algorithm will figure out how to center and zoom to fit this)
		setMarkerCoords(0, 0, sourceWidth, sourceHeight);
	} else {
		float xdim, ydim;
		// ok here we WANT a crop 
		if (sourceAspect > outAspect) {
			// we are longer aspect than the output, so we want to choose full height and crop out some of the sides
			ydim = ((float)sourceHeight / entryp->zoomLevel) * widthReduction;
			xdim = ydim * outAspect;
		}
		else {
			// we are taller than the output
			xdim = ((float)sourceWidth / entryp->zoomLevel) * heightReduction;
			ydim = xdim / outAspect;
		}
		float xmargin = (float)(sourceWidth - xdim) / 2.0f;
		float ymargin = (float)(sourceHeight - ydim) / 2.0f;
		setMarkerCoords((int) xmargin * entryp->alignxmod, (int) ymargin * entryp->alignymod, sourceWidth - (int)xmargin * (2-entryp->alignxmod), sourceHeight - (int)ymargin * (2-entryp->alignymod) );
	}
}
//---------------------------------------------------------------------------

