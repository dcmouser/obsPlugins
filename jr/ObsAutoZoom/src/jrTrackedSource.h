#pragma once

//---------------------------------------------------------------------------
// obs
#include <obs-module.h>
//
#include "jrPluginDefs.h"
//
#include "jrRegionDetector.h"
#include "jrMarkerlessManager.h"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// forward declarations to avoid recursive includes
class JrPlugin;
class SourceTracker;
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
class TrackedSource {
friend class SourceTracker;
public:
	SourceTracker* sourceTrackerp;
	JrPlugin* plugin;
	JrRegionDetector regionDetector;
	bool stageMemoryReady = false;

	// 6/18/23 bug on startup need to set true at start?
//	bool validForRender = false;
	bool validForRender = true;

	obs_weak_source_t* src_ref;
	char src_name[DefNameLenSource];
	int index;
	// for special filter owned first source
	bool externallyManaged;
	obs_source_t* externallySetSourcePointer;

	// dbx is the CURRENT DISPLAYED zoomcrop box -- it may be an in-flight moving box, progressing towards a target
	int lookingx1, lookingy1, lookingx2, lookingy2;
	// tbx is the instantaneously unstable current guessed location of marker box (we will not change bx to it until some stable delay)
	int markerx1, markery1, markerx2, markery2;
	// lastvalidmarkerx1 is the last good target before any occlusion; to be used if we lose sight of one of our two markers
	//int lastvalidmarkerx1, lastvalidmarkery1, lastvalidmarkerx2, lastvalidmarkery2;
	// current instant target under consideration
	//int targetx1, targety1, targetx2, targety2;
	// last cycles identified marker targets, useful for comparing to see how much the target is moving
	int lasttargetx1, lasttargety1, lasttargetx2, lasttargety2;
	// sticky location values which act as magnet targets while waiting for new target point to stabilize; so sticky may stay unchanged while instanteneous target candidate bounce around or change
	// note that sticky points are not nesc. the current location of current view (though often it will be); you can stick on a point you are moving to, and stickness keeps you going there.
	// we make these floats so we can more gradually drift them
	float stickygoalx1, stickygoaly1, stickygoalx2, stickygoaly2;
	// locations of last HUNT -- this helps us make sure we hunt when our position changes, even if its slow changes
	int lasthuntx1, lasthunty1, lasthuntx2, lasthunty2;

	int changeTargetMomentumCounter;
	int markerlessStreakCounter;
	int markerlessStreakCycleCounter;
	int settledLookingStreakCounter;
	//bool delayMoveUntilHuntFinishes;
	//
	JrRegionSummary lastGoodMarkerRegion1, lastGoodMarkerRegion2;
	int occludedCornerIndex;
	//
	bool markerBoxReady;
	bool markerBoxesExamined = false;
	bool lastTargetBoxReady;
	bool lookingBoxReady;
	bool stickyBoxReady;
	bool markerBoxIsOccluded;
	//
	int targetStableCount;
public:
	gs_texrender_t *stageHoldingSpaceTexrender;
	gs_texrender_t *sourceHoldingSpaceTexrender;
	gs_texrender_t *stagingTexrender;
	gs_texture_t *stageDrawingTexture;
	gs_stagesurf_t *stagingSurface;
	//
	uint8_t *stagedData;
	uint32_t stagedDataLineSize;
	//
	uint32_t sourceWidth;
	uint32_t sourceHeight;
	//
	int stageWidth, stageHeight;
	int stageArea;
	float stageShrinkX, stageShrinkY;
	float stageDistMult,stageDistMultOpt;
	//float stageAreaMult, stageAreaMultOpt;
	float sourceDistMult;
	//
	bool needsRecheckForSize;
	bool forceReallocate;
	//
	obs_source_t* sourceShowingIncdShowing;
public:
	void init(SourceTracker* st, int indexin);
	void prepareForUseAndSetName(const char *name);
	void clear();
	void freeForFinalExit();
	void freeAndClearForReuse();
	void freeGraphicMemoryUse();
public:
	bool hasValidWeakReference() { return (src_ref != NULL); };
	bool hasValidName() { return (src_name!=NULL && src_name[0]!=0); }
	//
	char* getName() { return src_name; };
	//void setName(const char* name) { strcpy(src_name, name); };
	void setWeakSourcep(obs_weak_source_t* wsp) { src_ref = wsp; };
	//
	JrPlugin* getPluginp() { return plugin;	};
	//
	void calculateSourceSize(obs_source_t* source, uint32_t& width, uint32_t& height);
	uint32_t getSourceWidth() { return sourceWidth; };
	uint32_t getSourceHeight() { return sourceHeight; };
	//
	void clearAllBoxReadies();
	void clearFoundTrackingBox();
	void setStickyTargetToMarkers();
	bool isRegionSimilarBounds(JrRegionSummary* regiona, JrRegionSummary* regionb);
	bool isOneValidRegionAtUnchangedLocation();
	//
	void setMarkerCoords(int x1, int y1, int x2, int y2) { markerx1 = x1; markery1 = y1; markerx2 = x2; markery2 = y2; };
public:
	obs_weak_source_t* getWeakSourcep() { return src_ref; };
	obs_source_t* borrowFullSourceFromWeakSource();
	void releaseBorrowedFullSource(obs_source_t* sourcep);
protected:
	void freeWeakSource();
	void freeFullSource(obs_source_t* src);
public:
	void updateZoomCropBoxFromCurrentCandidate(bool debugPreviewOnly);
	bool findNewCandidateTrackingBox(bool debugPreviewOnly);
public:
	void onOptionsChange();
public:
	bool allocateGraphicsData();
	void freeBeforeReallocateTexRender();
	void freeBeforeReallocateFilterTextures();
	void freeBeforeReallocateNonGraphicData();
	bool recheckSizeAndAdjustIfNeeded(obs_source_t* source);
	void reallocateGraphiocsForTrackedSource(obs_source_t* source);
public:
	JrRegionDetector* getRegionDetectorp() { return &regionDetector; }
	void markRegionsQuality();
	bool calcIsValidmarkerRegion(JrRegionSummary* region);
	//
	void analyzeSceneAndFindTrackingBox(uint8_t *data, uint32_t dlinesize, bool isHunting, bool debugPreviewOnly);
	void analyzeSceneFindComponentMarkers(uint8_t *data, uint32_t dlinesize);
	void setStageSize(int instageWidth, int instageHeight);
	//
	void doRenderWorkFromEffectToStageTexRender(obs_source_t* source);
	bool doRenderWorkFromStageToInternalMemory();
	void overlayDebugInfoOnInternalDataBuffer();
	void findTrackingMarkerRegionInSource(obs_source_t* source, bool shouldUpdateTrackingBox, bool isHunting, bool debugPreviewOnly);
public:
	void doRenderAutocropBoxToScreen(obs_source_t* source, int owidth, int oheight);
	void doRenderAutocropBoxFadedToScreen(obs_source_t* source, int owidth, int oheight, TrackedSource* fromtsp, float fadePosition);
	void doRenderAutocropBoxIntoTextureFromHoldingSpace(gs_texrender_t* texRender, int owidth, int oheight);
public:
	bool areMarkersBothVisibleNeitherOccluded() { return markerBoxReady && !markerBoxIsOccluded; };
	bool areMarkersBothVisibleOrOneOccluded() { return markerBoxReady; };
	bool areMarkersMissing() { return !markerBoxReady; }
	bool areMakersExamined() { return markerBoxesExamined; };
public:
	void updateAfterHuntCheck();
public:
	void touchRefreshDuringRenderCycle();
	void touchKludgeDuringNonRenderTickToKeepAlive();
public:
	void setExternallyManagedSource(obs_source_t* sourcep);
public:

	void travelToTrackedSourceMarkerless(JrMarkerlessEntry *entryp, bool forceInstant, bool useFade);
	void travelToSourceCoords(int x1, int y1, int x2, int y2, bool forceInstant, bool useFade);
	void setMarkerCoordsToBoundaries();
	void setMarkerCoordsFromMarklessEntry(JrMarkerlessEntry* entryp);
	//
	void setLocationsToMarkerLocations(bool flagInstantLookingPos);
public:
	void calculateUpdateStageSize();
public:
	int cnvStageToSourceX(int stagex) { return (int)(stageShrinkX * stagex); }
	int cnvStageToSourceY(int stagey) { return (int)(stageShrinkY * stagey); }
	int cnvStageToSourceGeneric(int stagex) { return (int)(stageShrinkX * stagex); }
	int cnvSourceToStageX(int sourcex) { return (int)((float)sourcex / (float)stageShrinkX); }
	int cnvSourceToStageY(int sourcey) { return (int)((float)sourcey / (float)stageShrinkY); }
	int cnvSourceToStageGeneric(int sourcex) { return (int)((float)sourcex / (float)stageShrinkX); }
public:
	float calcMinDistanceBetweenRegions(JrRegionSummary* region1, JrRegionSummary* region2);
	float calcWorstRegionPointDist(JrRegionSummary* regiona, JrRegionSummary* regionb);
public:
	bool gs_stage_texture_sanityCheck(gs_stagesurf_t* src, gs_texture_t* texSrc);
	bool jrcan_stage(struct gs_stage_surface* dst, struct gs_texture_2d* src);
public:
	void doRenderToInternalMemoryPostProcessing();
public:
	void doRenderFromInternalMemoryToFilterOutput(int outx1, int outy1, int outWidth, int outHeight);
	void doRenderSourceWithInternalMemoryToFilterOutput(int outx1, int outy1, int outWidth, int outHeight, bool optionShowOnTopOfSource);
//
	gs_texrender_t* doPipeLineOutputEffect_Blur(gs_texrender_t* inputTexRender, int owidth, int oheight);
public:
	void resetMarkerLessCounters();
public:
	void doRender_Dilate_Effect_OnStagingTexrender(int dilateGreenSteps, int dilateRedSteps);
	void doRender_Dilate_Effect_OnStagingTexrender_ColorToColor(vec4& colorFrom, vec4& colorTo);
public:
	void setMarkerBoxReady(bool readyVal, bool examinedVal) { markerBoxReady = readyVal; markerBoxesExamined = examinedVal; };
	bool isViewNearLocation(int x1, int y1, int x2, int y2);
};



