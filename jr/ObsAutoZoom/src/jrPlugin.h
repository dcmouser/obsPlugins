#pragma once

//---------------------------------------------------------------------------
#include <windows.h>
#include <ctime>
#include <string>
//
// obs
#include <obs-module.h>
//
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
//
//#include "obsHelpers.h"
//
#include "jrPluginDefs.h"
//
#include "jrSourceTracker.h"
#include "jrMarkerlessManager.h"
//
//#include "source_list.h"
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
enum EnumJrPluginType {EnumJrPluginTypeUnknown, EnumJrPluginTypeSource, EnumJrPluginTypeFilter};
extern const char* markerMultiColorModeRenderTechniques[];
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// GLOBAL CHAR* ARRAYS USED IN SETTINGS PROPERTIES (would be better to have these as simple defines but awkward)
extern const char* SETTING_zcMarkerPos_choices[];
extern const char* SETTING_zcAlignment_choices[];
extern const char* SETTING_zcMode_choices[];
extern const char* SETTING_zcCropStyle_choices[];
extern const char* SETTING_zcEasing_choices[];
extern const char* SETTING_fadeMode_choices[];
extern const char* SETTING_markerMultiColorMode_choices[];
extern const char* SETTING_zcKeyMode_choices[];
extern const char* SETTING_zcKeyColor_choicesFull[];
extern const char* SETTING_zcKeyColor_choicesReduced[];
extern const char* SETTING_zcMarkerlessMode_choices[];
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
#define manualZoomInOutHoldResistanceTarget 1
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class JrPlugin {
public:
	HANDLE mutex;
	SourceTracker stracker;
public:
	EnumJrPluginType pluginType;
	// obs pointer
	obs_source_t *context;
public:
	gs_texrender_t* outTexRenderA;
	gs_texrender_t* outTexRenderB;
	gs_texrender_t* outTexRenderBase;
	gs_texrender_t* fadeTexRenderA;
	gs_texrender_t* fadeTexRenderB;
public:
	// modified chroma effect
	gs_effect_t *effectChromaKey;
	// chroma effect params
	gs_eparam_t *chroma_pixel_size_param;
	//
	gs_eparam_t *chroma1_param;
	gs_eparam_t *similarity1_param;
	gs_eparam_t *smoothness1_param;
	//
	gs_eparam_t *chroma2_param;
	gs_eparam_t *similarity2_param;
	gs_eparam_t *smoothness2_param;
	//
	gs_eparam_t *testThreshold_param;
public:
	// modified chroma effect
	gs_effect_t *effectHsvKey;
	// chroma effect params
	gs_eparam_t *hsv_pixel_size_param;
	//
	gs_eparam_t* color1hsv_param;
	gs_eparam_t* color2hsv_param;
	gs_eparam_t *hueThreshold1_param;
	gs_eparam_t *saturationThreshold1_param;
	gs_eparam_t *valueThreshold1_param;
	gs_eparam_t *hueThreshold2_param;
	gs_eparam_t *saturationThreshold2_param;
	gs_eparam_t *valueThreshold2_param;
public:
	// internal params for chroma effect
	int opt_markerMultiColorMode;
	uint32_t opt_chroma_color1, opt_chroma_color2;
	struct vec2 opt_chroma1, opt_chroma2;
	float opt_similarity1, opt_similarity2;
	float opt_smoothness1, opt_smoothness2;
	//float opt_hsvTestThreshold1, opt_hsvTestThreshold2;
	int opt_dilateGreenSteps, opt_dilateRedSteps;
	float opt_testThreshold;
	int opt_chromaDualColorGapFill;
	//
	uint32_t opt_hsv_color1, opt_hsv_color2;
	float opt_hueThreshold1;
	float opt_saturationThreshold1;
	float opt_valueThreshold1;
	float opt_hueThreshold2;
	float opt_saturationThreshold2;
	float opt_valueThreshold2;
	//
	struct vec3 color1AsHsv, color2AsHsv;
	struct vec4 colorGreenAsRgbaVec, colorRedAsRgbaVec, colorBackgroundAsRgbaVec;
	struct vec4 colorGreenTempAsRgbaVec, colorRedTempAsRgbaVec;
public:
	// zoomCrop effect
	gs_effect_t *effectZoomCrop;
	// zoomCrop effect params
	gs_eparam_t *param_zoom_mul;
	gs_eparam_t *param_zoom_add;
	gs_eparam_t* param_zoom_clip_ul;
	gs_eparam_t* param_zoom_clip_lr;
	gs_eparam_t* param_zoom_hardClip_ul;
	gs_eparam_t* param_zoom_hardClip_lr;
	gs_eparam_t *param_zoom_pixel_size;
	// internal params
	struct vec2 ep_zoom_mulVal;
	struct vec2 ep_zoom_addVal;
	struct vec2 ep_zoom_clip_ul;
	struct vec2 ep_zoom_clip_lr;
	struct vec2 ep_zoom_hardClip_ul;
	struct vec2 ep_zoom_hardClip_lr;
public:
	// fade effect
	gs_effect_t *effectFade;
	gs_eparam_t* param_fade_a;
	gs_eparam_t* param_fade_b;
	gs_eparam_t* param_fade_val;
public:
	// dilate effect
	gs_effect_t* effectDilate;
	gs_eparam_t* param_dilate_pixel_size;
	gs_eparam_t *param_dilateColorFromRgba;
	gs_eparam_t *param_dilateColorToRgba;
	gs_eparam_t *param_dilateBackgroundRgba;
public:
	// output effects
	gs_effect_t *effectOutput;
	gs_eparam_t *param_output_pixel_size;
	gs_eparam_t* param_output_clip_ul;
	gs_eparam_t* param_output_clip_lr;
	gs_eparam_t* param_output_hardClip_ul;
	gs_eparam_t* param_output_hardClip_lr;
	gs_eparam_t* param_output_effectInside;
	gs_eparam_t* param_output_passNumber;
	gs_eparam_t* param_ouput_secondaryTex;
	gs_eparam_t* param_ouput_exteriorDullness;
	struct vec2 ep_output_clip_ul;
	struct vec2 ep_output_clip_lr;
	struct vec2 ep_output_hardClip_ul;
	struct vec2 ep_output_hardClip_lr;
	//
	float ep_optBlurExteriorDullness;
	int ep_optBlurPasses;
	float ep_optBlurSizeReduction;

public:
	// options
	bool opt_zcPreserveAspectRatio;
	//
	int opt_zcMarkerPos;
	int opt_zcBoxMargin;
	int opt_zcBoxMoveSpeed;
	int opt_zcReactionDistance;
	int opt_zcBoxMoveDelay;
	int opt_zcMissingMarkerTimeout;
	int opt_zcValidMarkersToTestForOcclusion;
	//
	// general configurable params
	bool opt_filterBypass;
	//
	bool opt_debugRegions;
	bool opt_debugChroma;
	bool opt_debugAllUpdate;
	//
	bool opt_autoTrack;
	int opt_trackRate;
	//
	float opt_rmTDensityMin;
	float opt_rmTAspectMin;
	int opt_rmSizeMin;
	int opt_rmSizeMax;
	//
	int opt_keyMode;
	float opt_rmMinColor2Percent;
	float opt_rmMaxColor2Percent;
	//
	int opt_rmStageSize;
	int opt_rmTooCloseDist;
	//
	// cropping aspect things
	int opt_zcAlign;
	int opt_zcMode;
	float opt_zcMaxZoom;
	char opt_OutputSizeBuf[80];
	float forcedOutputAspect;
	int opt_zcEasing;
	int opt_fadeMode;
	int opt_zcCropStyle;
	char cropZoomTechnique[16];

public:
	// markerless stuff - for when we can't find markers, do we show entire view or some subset
	int opt_markerlessMode = 0;
	//
	bool opt_enableAutoSourceHunting;
	int opt_manualViewSourceIndex;
	//
	char opt_markerlessCycleListBuf[DefMarkerlessCycleListBufMaxSize];
	int opt_markerlessCycleIndex;
	int opt_manualZoomSourceIndex = 0;
	float opt_manualZoomSourceScale = 1.0f;
	float opt_manualZoomStepSize = 0.10f;
	float opt_manualZoomTransitions[DefMaxSources];
	int opt_manualZoomAlignment;
	char opt_manualZoomTransitionsString[80];
	float opt_manualZoomMinZoom = 0.5;
	//
	// see pluginCodeMisc.cpp; we want to hold at max+min zoom levels for a hotkey press or two
	//const int manualZoomInOutHoldResistanceTarget = 1;
	int manualZoomInOutHoldCount = 0;
	int manualZoomInOutHoldDirection = 0;
public:
	int kludgeTouchCounter;
	int kludgeTouchCounterHidden;

	bool opt_avoidTrackingInTransitions;
	float opt_fadeDuration;

	// size
	uint32_t outputWidthAutomatic;
	uint32_t outputHeightAutomatic;
	int outputWidthPlugin;
	int outputHeightPlugin;

	// hotkeys
	obs_hotkey_id hotkeyId_ToggleAutoUpdate;
	obs_hotkey_id hotkeyId_OneShotZoomCrop;
	obs_hotkey_id hotkeyId_ToggleCropping, hotkeyId_ToggleCropBlurMode;
	obs_hotkey_id hotkeyId_ToggleDebugDisplay;
	obs_hotkey_id hotkeyId_ResetView;
	obs_hotkey_id hotkeyId_ToggleLastGoodMarkers;
	obs_hotkey_id hotkeyId_CycleSource, hotkeyId_CycleSourceBack;
	obs_hotkey_id hotkeyId_CycleViewForward, hotkeyId_CycleViewBack;
	obs_hotkey_id hotkeyId_toggleAutoSourceHunting;

	// to keep tracking of update rate and one-shot adjustment
	int trackingUpdateCounter;
	//
	bool in_enumSources;
	//
	bool sourcesHaveChanged;
	//
	// computed options
	float computedChangeMomentumDistanceThresholdMoving;
	float computedChangeMomentumDistanceThresholdStabilized;
	float computedThresholdTargetStableDistance;
	int computedThresholdTargetStableCount;
	float computedChangeMomentumDistanceThresholdDontDrift;
	float computedAverageSmoothingRateToCloseTarget;
	bool computedIntegerizeStableAveragingLocation;
	int computedMomentumCounterTargetMissingMarkers;
	int computedMomentumCounterTargetNormal;
	float computedMinDistBetweenMarkersIsTooClose;
	//
	bool forceAllAnalyzeMarkersOnNextRender;
	//
	bool currentlyTransitioning = false;
public:
	int fadeStartingSourceIndex, fadeEndingSourceIndex;
	float fadePosition;
	clock_t fadeStartTime;
	clock_t fadeEndTime;
	clock_t fadeDuration;
	bool firstRender;
	//
	clock_t oneShotEndTime;
	bool oneShotEngaged;
	int oneShotStage;
	bool oneShotFoundTarget;
	bool oneShotDidAtLeastOneTrack;
	//
	bool sourceDetailsHaveChanged;

public:
	EnumJrPluginType getPluginType() { return pluginType; }
	bool isPluginTypeFilter() { return true && (pluginType == EnumJrPluginTypeFilter); }
	const char* getPluginIdCharp() { return obs_source_get_id(context); }
	//
	// note sure if this is right way to get the source represented by this plugin
	obs_source_t* getThisPluginSource() { return context; }
	obs_source_t* getThisPluginFiltersAttachedSource() { return isPluginTypeFilter() ? obs_filter_get_parent(context) : NULL; };
	const char* getThisPluginSourceName() { return obs_source_get_name(context); }
	//
	int getPluginOutputWidth() { return outputWidthPlugin; };
	int getPluginOutputHeight() { return outputHeightPlugin; };
	int getPluginOutputWidthAutomatic() { return outputWidthAutomatic; };
	int getPluginOutputHeightAutomatic() { return outputHeightAutomatic; };
	//
	bool enableAutoSwitchingSources() { return (opt_enableAutoSourceHunting && !opt_filterBypass); };
	//
	int getOptMarkerMultiColorMode() { return opt_markerMultiColorMode; };
	const char* getOptMarkerMultiColorModeStr() { return markerMultiColorModeRenderTechniques[opt_markerMultiColorMode]; };
	bool getOptKeyMode() { return opt_keyMode; }
public:
	obs_properties_t* doPluginAddProperties();
	void updateSettingsOnChange(obs_data_t* settings);
	void updateCropStyleDrawTechnique();
	static void doGetPropertyDefauls(obs_data_t* settings);
public:
	void freeBeforeReallocateEffects();
	bool initFilterInGraphicsContext();
	bool initFilterOutsideGraphicsContext();
	//
	void doTick();
	void doRender();
	//
	void doShow();
	void doHide();

	bool doCalculationsForZoomCropEffect(TrackedSource* tsourcep);

	void parseTextCordsString(const char* coordStrIn, int* x1, int* y1, int* x2, int* y2, int defx1, int defy1, int defx2, int defy2, int maxwidth, int maxheight);
	int parseCoordStr(const char* cpos, int max);
	//
	void forceUpdatePluginSettingsOnOptionChange();
	//
	void updateOutputSize();
	//
	void reRegisterHotkeys();
	void addPropertyForASourceOption(obs_properties_t* pp, const char* name, const char* desc);
	//
	void setEffectParamsChromaKey(uint32_t swidth, uint32_t sheight);
	void setEffectParamsHsvKey(uint32_t swidth, uint32_t sheight);
	void setEffectParamsZoomCrop(uint32_t swidth, uint32_t sheight);
	void setEffectParamsFade(uint32_t swidth, uint32_t sheight);
	void setEffectParamsOutput(TrackedSource* tsourcep, uint32_t swidth, uint32_t sheight, int passNumber, gs_texrender_t* secondaryTexture);
	void setEffectParamsDilate(uint32_t swidth, uint32_t sheight);
	//
	SourceTracker* getSourceTrackerp() { return &stracker; };
	void multiSourceTargetLost();
	void multiSourceTargetChanged();
	//
	void saveCurrentViewedSourceAsManualViewOption() { opt_manualViewSourceIndex = stracker.getViewSourceIndex(); };
public:
	void updateComputedOptions();
	bool updateComputeAutomaticOutputSize();
public:
	void onSourcesHaveChanged();
	void setSourcesChanged(bool val) { sourcesHaveChanged = val; }
public:
	void initiateFade(int startingSourceIndex, int endingSourceIndex);
	void cancelFade();
	bool updateFadePosition();
	bool isFading() { return (fadeStartingSourceIndex != -1); }
public:
	void goToInitialView();
public:
	void initiateOneShot();
	void cancelOneShot();
	void updateOneShotStatus(bool isStill, bool goodMarkersFound);
	bool isOneShotEngaged() { return oneShotEngaged; };
	bool isOneShotEngagedAndFirstStage() { return (oneShotEngaged && oneShotStage == 0); };
	bool isOneShotEngagedButPastFirstStage() { return (oneShotEngaged && oneShotStage > 0); };
	bool didOneShotFindValidTarget() { return oneShotFoundTarget; };
	void setOneShotFoundValidTarget(bool val) { oneShotFoundTarget = val; };
public:
	void saveVolatileSettings();
	void saveVolatileMarkerZoomScaleSettings(bool isCustomColor);
public:
	void gotoCurrentMarkerlessCoordinates();
	void gotoLastGoodMarkerLocation();
public:
	bool updateMarkerlessSettings();
	bool getMarkerlessModeIsPresets() { return opt_markerlessMode == 1; }
	bool getMarkerlessModeIsManualZoom() { return opt_markerlessMode == 0; }
	bool getMarkerlessModeIsDisabled() { return opt_markerlessMode == 2; }
public:
	void performViewCycleAdvance(int delta);
	void markerlessCycleListAdvance(int delta);
	void viewingSourceManuallyAdvance(int delta);
	//bool gotoManualViewSourceIndex();
	//bool advanceManualViewSourceIndex(int delta);
public:
	void notifySourceSizeChanged(TrackedSource* tsp) { sourceDetailsHaveChanged = true; };
	void adjustToSourceSizeChanges(bool flagForce);
	void convertStageSizeIndexToDimensions(int stageSizeIndex, int& width, int& height);
	void computeStageSizeMaxed(int& width, int& height, int maxWidth, int maxHeight);
public:
	bool calibrateMarkerSizes();
	bool calibrateMarkerChroma();
	bool calibrateSourceZoomScales();
public:
	void handleHotkeyPress(obs_hotkey_id id, obs_hotkey_t* key);
public:
	bool checkIsTransitioning();
public:
	void doManualZoomInOutByPercent(float scalePercent);
	void fillFloatListFromString(char* commaList, float* floatList, int max);
public:
	void performToggleAutoUpdate();
	void performToggleLastGoodMarkers();
	void performToggleAutoSourceHunting();
	void performInitiateOneShot();
	void performToggleCropping();
	void performCycleCropBlurMode();
	void performToggleDebugDisplay();
	void performResetView();
public:
	bool isThisPluginSourceActiveAndVisible();
	obs_source_t* findActiveAndVisibleAutoZoomSource();
	obs_source_t* findActiveAndVisibleAutoZoomSourceInScene(obs_scene_t* scenep);
	obs_source_t* findActiveAndVisibleAutoZoomSourceInSceneUsingEnum(obs_scene_t* scenep);
public:
	bool triggerVisibleActionSignal(uint32_t actionSignalKey);
	bool sendVisibleActionSignal(uint32_t actionSignalKey);
	bool receiveVisibleActionSignal(uint32_t actionSignalKey);
public:
	void touchKludgeAllSourcesOnHidden();
public:
	void adjustChildSourcesVisibility(bool isVisible);
public:
	bool doImport(bool flagPreserveDisplayOptions);
	bool doExport();
	std::string getModuleConfigPath();
	void cleanOldUnusedSettingsForSource();
};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// static
bool findAutoZoomSourceFromSceneItemEnum(obs_scene_t* scene, obs_sceneitem_t* item, void* param);
//---------------------------------------------------------------------------
