//---------------------------------------------------------------------------
#include "jrSourceTracker.h"
#include "jrPlugin.h"
#include "obsHelpers.h"
#include "jrfuncs.h"
//---------------------------------------------------------------------------

































//---------------------------------------------------------------------------
// reaallocate staging memory stuff if needed
bool TrackedSource::recheckSizeAndAdjustIfNeeded(obs_source_t* source) {
	JrPlugin* plugin = getPluginp();

	//mydebug("In TrackedSource::recheckSizeAndAdjustIfNeeded with index: %d.", index);

	// reset flag since we are doing so now
	needsRecheckForSize = false;

	// update if size changes
	bool update = false;

	if (!source) {
		// clear EVERYTHING if we can't find source (this will not yet free any memory...)
		clear();
		//
		// is this part really needed? or will we do this when we need to resize..
		if (true) {
			obs_enter_graphics();
			freeBeforeReallocateFilterTextures();
			obs_leave_graphics();
			freeBeforeReallocateNonGraphicData();
		}
		return false;
	}

	// now we can get size
	int oldSourceWidth = sourceWidth;
	int oldSourceHeight = sourceHeight;
	int oldStageWidth = stageWidth;
	int oldStageHeight = stageHeight;
	calculateSourceSize(source, sourceWidth, sourceHeight);
	// calculate our stage sizes etc
	calculateUpdateStageSize();

	//mydebug("recheckSizeAndAdjustIfNeeded 2 with new dimensions %dx%d vs old of %dx%d.", newSourceWidth, newSourceHeight, sourceWidth, sourceHeight);

	if (forceReallocate || oldSourceWidth != sourceWidth || oldSourceHeight != sourceHeight || oldStageWidth != stageWidth || oldStageHeight != stageHeight) {
		update = true;
		forceReallocate = false;

		//mydebug("ATTN: jr - Ok size has changed for index %d, we need to reallocate old size is %d,%d - %d,%d and new size is %d,%d - %d,%d.", index, sourceWidth, sourceHeight, stageWidth, stageHeight, newSourceWidth, newSourceHeight, newStageWidth, newStageHeight);
		//mydebug("ATTN: jr - in recheckSizeAndAdjustIfNeeded and resizing..");

		// mutex surround -- not sure if needed
		WaitForSingleObject(plugin->mutex, INFINITE);

		// always true
		bool sizeChanged = true;

		// mark current boxes as invalid
		if (sizeChanged) {
			plugin->notifySourceSizeChanged(this);
			clearAllBoxReadies();
		}

		// now we need to reallocate graphics
		reallocateGraphiocsForTrackedSource(source);

		// source tracker and region detect helper
		setStageSize(stageWidth, stageHeight);

		// done in protected mutex area
		ReleaseMutex(plugin->mutex);

		//mydebug("Done reallocating space for tracked source.");
	}

	// success
	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TrackedSource::calculateSourceSize(obs_source_t* source, uint32_t &width, uint32_t &height) {
	width = jrSourceCalculateWidth(source);
	height = jrSourceCalculateHeight(source);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void TrackedSource::reallocateGraphiocsForTrackedSource(obs_source_t* source) {
		obs_enter_graphics();
		freeBeforeReallocateFilterTextures();
		// allocate staging area based on DESTINATION size?
		stagingSurface = gs_stagesurface_create(stageWidth, stageHeight, GS_RGBA);
		// new drawing texture
		stageDrawingTexture = gs_texture_create(stageWidth, stageHeight, GS_RGBA, 1, NULL, GS_DYNAMIC);
		obs_leave_graphics();

		//info("JR Created drawing and staging texture %d by %d (%d x %d).", sourceWidth, sourceHeight, stageWidth, stageHeight);

		// ATTN: IMPORTANT - this bug this is making current code blank
		freeBeforeReallocateNonGraphicData();

		// realloc internal memory
		stagedData = (uint8_t *) bzalloc((stageWidth + 32) * stageHeight * 4);

		// experimental 12/24/22 -- tell obs we are showing this source
		if (false) {
			obs_source_inc_showing(source);
			sourceShowingIncdShowing = source;
		}
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void TrackedSource::calculateUpdateStageSize() {
	/// calculate and update all stage related size variables
	JrPlugin* plugin = getPluginp();

	// calculate stage size
	plugin->computeStageSizeMaxed(stageWidth, stageHeight, sourceWidth, sourceHeight);

	// and now helper calculations that we use later
	stageArea = stageWidth * stageHeight;
	stageShrinkX = (float)sourceWidth / (float)stageWidth;
	stageShrinkY = (float)sourceHeight / (float)stageHeight;
	//
	// to help us scale calculations for options based on stage size -- the bigger the stage the bigger the multiplier
	// we use this on all of our calculation of distance regarding markers
	// ATTN: to improve;
	//stageDistMult = (float)stageWidth / 480.0f;
	// so let's scale it to be apx pixels on a standard 1920x1080 monitor
	stageDistMult = (float)stageWidth / 1920.0f;
	//
	// ATTN:TODO - lets reconsider using stageAreaMult
	//stageAreaMult = ((float)stageWidth / 480.0f) * ((float)stageHeight / 270.0f);
	//
	// and then this one modified by users option per source
	float sourceStatMultiplierOption = sourceTrackerp->getSourceMarkerZoomScale(index);
	if (sourceStatMultiplierOption == 0) {
		sourceStatMultiplierOption = 1.0f;
	}
	//mydebug("Using for index %d sourceStatMultiplierOption = %f.", index, sourceStatMultiplierOption);

	stageDistMultOpt = stageDistMult * sourceStatMultiplierOption;
	//stageAreaMultOpt = stageAreaMult * sourceStatMultiplierOption;
	//
	// simple scaling for source size compared to a standard 1920x
	sourceDistMult = sourceWidth==0 ? 1.0f : (float)sourceWidth / 1920.0f;

	//mydebug("In calculateUpdateStageSize for index %d with stageDistMultOpt = %f and %f.", stageDistMultOpt, sourceDistMult);
}
//---------------------------------------------------------------------------





