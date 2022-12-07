#pragma once

//---------------------------------------------------------------------------
// obs
#include <obs-module.h>
//
#include "jrPluginDefs.h"
//
#include "jrRegionDetector.h"
#include "jrTrackedSource.h"
#include "jrMarkerlessManager.h"
//
#include <ctime>
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// forward declarations to avoid recursive includes
class JrPlugin;
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
enum EnumHuntType { EnumHuntType_None, EnumHuntType_NewTargetPushIn, EnumHuntType_TargetLostPullOut, EnumHuntType_LongTimeMarkerless, EnumHuntType_LongTimeStable };
//---------------------------------------------------------------------------


class SourceTracker {
public:
	JrPlugin* plugin;
	TrackedSource tsource[DefMaxSources];
	JrMarkerlessManager markerlessManager;
public:
	JrMarkerlessEntry manualZoomEntry;
public:
	float sourceZoomScale[DefMaxSources];
public:
	// auto switching between sources (cameras)
	int huntDirection;
	int huntIndex;
	int huntStopIndex;
	bool huntingWider;
	enum EnumHuntType huntType;
	bool shouldConcludeNoMarkersAnywhereAndSwitch;
public:
	int sourceCount;
	int sourceIndexViewing;
	int sourceIndexTracking;
public:
	clock_t trackingDelayEndTime;
public:
	void init(JrPlugin* inpluginp);
	void freeForFinalExit();

	JrPlugin* getPluginp() { return plugin; };
	//
	int getViewSourceIndex() { return sourceIndexViewing; }
	int getTrackSourceIndex() { return sourceIndexTracking; }
	TrackedSource* getCurrentSourceViewing() { return &tsource[sourceIndexViewing]; };
	TrackedSource* getCurrentSourceTracking() { return &tsource[sourceIndexTracking]; };
	TrackedSource* getTrackedSourceByIndex(int ix)  { return &tsource[ix]; };
	int getSourceCount() { return sourceCount; };
	obs_weak_source_t* getWeakSourcepByIndex(int ix) { return tsource[ix].src_ref; };
	void setViewSourceIndex(int ix, bool useFade);
	void setTrackSourceIndex(int ix) { sourceIndexTracking = ix; }
	//
	obs_source_t* borrowFullSourceFromWeakSourcepByIndex(int ix) { return tsource[ix].borrowFullSourceFromWeakSource(); };
	//
	void reviseSourceCount(int reserveSourceCount);
	void updateSourceIndexByName(int ix, const char* name);
	//
	bool checkAndUpdateAllTrackingSources();
	bool checkAndUpdateTrackingSource(TrackedSource* tsourcep);
public:
	void setSourceMarkerZoomScale(int ix, float val) { sourceZoomScale[ix] = val; };
	float getSourceMarkerZoomScale(int ix) { return sourceZoomScale[ix]; };
public:
	void clearAllBoxReadies();
	void onOptionsChange();
public:
	void getDualTrackedViews(TrackedSource* &tsourcepView, TrackedSource* &tsourcepTrack);
	//
	void initiateHunt(EnumHuntType htype, bool inshouldConcludeNoMarkersAnywhereAndSwitch);
	void internalInitiateHuntOnNewTargetFoundPushIn();
	void internalInitiateHuntOnTargetLostPullOut();
	//
	bool AdvanceHuntToNextSource();
	bool isHunting() { return (huntDirection != 0); };
	void foundGoodHuntedSourceIndex(int huntingIndex);
	void abortHunting();
	void abortHuntingIfPullingWide();
	void setTrackingIndexToHuntIndex();
	//
	void onHuntFinished();
	void onPullOutHuntFailedToFindMarkersAnywhere();
	//
	void setExternallyManagedTrackedSource(int index, obs_source_t* sourcep);
public:
	void travelToMarkerlessDefault() { travelToMarkerless(-1, false, true); };
	void travelToMarkerlessStayOnViewingSource() { travelToMarkerless(getViewSourceIndex(), false, true); };
	void travelToMarkerlessStayOnSource(int ix) { travelToMarkerless(ix, false, true);  };
	void travelToMarkerless(int forcedSourceId, bool forceInstant, bool useFade);
	void travelToMarkerlessInitiate(bool bypassDisabledAutoSourceHuntingOption);
	void reTravelToMarkerlessIfMarkerless(bool bypassDisabledAutoSourceHuntingOption);
	void goDirectlyToMakersAndDelayHunting();
public:
	//void gotoMarkerlessPosition(bool flagAllowChangeSource);
	bool parseSettingString(const char* settingBuf);
	JrMarkerlessEntry* lookupMarkerlessEntry(int entryStartIndex, int sourceIndexRequired);
public:
	void calculateMaxSourceDimensions(int &width, int &height);
public:
	void delayHuntingBriefly();
	void cancelDelayHunting();
	bool isTrackingDelayed();
public:
	void updateManualZoomEntry(int sourceIndex, float zoomLevel, int alignmentMode);
};
//---------------------------------------------------------------------------

