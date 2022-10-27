#pragma once

//---------------------------------------------------------------------------
#include <windows.h>
#include <ctime>
//
// obs
#include <obs-module.h>
//
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
//
#include "obsHelpers.h"
//
#include "jrPluginDefs.h"
//
#include "jrSourceTracker.h"
#include "jrMarkerlessManager.h"
//
#include "source_list.h"
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
enum EnumJrPluginType {EnumJrPluginTypeUnknown, EnumJrPluginTypeSource, EnumJrPluginTypeFilter};
extern char* markerChromaModeRenderTechniques[];
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// GLOBAL CHAR* ARRAYS USED IN SETTINGS PROPERTIES (would be better to have these as simple defines but awkward)
extern char* SETTING_zcMarkerPos_choices[];
extern char* SETTING_zcAlignment_choices[];
extern char* SETTING_zcMode_choices[];
extern char* SETTING_zcCropStyle_choices[];
extern char* SETTING_zcEasing_choices[];
extern char* SETTING_fadeMode_choices[];
extern char* SETTING_markerChromaMode_choices[];
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
	gs_effect_t *effectChroma;
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
	gs_eparam_t *chromaThreshold_param;
	// internal params for chroma effect
	int opt_markerChromaMode;
	uint32_t opt_key_color1, opt_key_color2;
	struct vec2 opt_chroma1, opt_chroma2;
	float opt_similarity1, opt_similarity2;
	float opt_smoothness1, opt_smoothness2;
	float opt_chromaThreshold;
	int opt_chromaDualColorGapFill;
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
	//
	// general configurable params
	bool opt_filterBypass;
	bool opt_ignoreMarkers;
	bool opt_resizeOutput;
	//
	bool opt_debugRegions;
	bool opt_debugChroma;
	bool opt_debugAllUpdate;
	//
	bool opt_enableAutoUpdate;
	int opt_updateRate;
	//
	float opt_rmTDensityMin;
	float opt_rmTAspectMin;
	int opt_rmSizeMin;
	int opt_rmSizeMax;
	//
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

	bool opt_enableMarkerlessUse;
	bool opt_enableAutoSourceHunting;
	int opt_manualViewSourceIndex;

	// markerless stuff - for when we can't find markers, do we show entire view or some subset
	char opt_markerlessCycleListBuf[DefMarkerlessCycleListBufMaxSize];
	int opt_markerlessCycleIndex;
	float opt_fadeDuration;
	int kludgeTouchCounter;

	// size
	uint32_t outputWidthAutomatic;
	uint32_t outputHeightAutomatic;
	int outputWidthPlugin;
	int outputHeightPlugin;

	// hotkeys
	obs_hotkey_id hotkeyId_ToggleAutoUpdate;
	obs_hotkey_id hotkeyId_OneShotZoomCrop;
	obs_hotkey_id hotkeyId_ToggleCropping;
	obs_hotkey_id hotkeyId_ToggleDebugDisplay;
	//obs_hotkey_id hotkeyId_ToggleBypass;
	obs_hotkey_id hotkeyId_ToggleIgnoreMarkers;
	obs_hotkey_id hotkeyId_CycleSource, hotkeyId_CycleSourceBack;
	obs_hotkey_id hotkeyId_CycleViewForward, hotkeyId_CycleViewBack;
	obs_hotkey_id hotkeyId_toggleEnableMarkerlessUse;
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
	//
	int getPluginOutputWidth() { return outputWidthPlugin; };
	int getPluginOutputHeight() { return outputHeightPlugin; };
	int getPluginOutputWidthAutomatic() { return outputWidthAutomatic; };
	int getPluginOutputHeightAutomatic() { return outputHeightAutomatic; };
	//
	bool enableAutoSwitchingSources() { return (opt_enableAutoSourceHunting && !opt_filterBypass && !opt_ignoreMarkers); };
	//
	int getOptMarkerChromaMode() { return opt_markerChromaMode; };
	char* getOptMarkerChromaModeStr() { return markerChromaModeRenderTechniques[opt_markerChromaMode]; };
public:
	obs_properties_t* doPluginAddProperties();
	void updateSettingsOnChange(obs_data_t* settings);
	static void doGetPropertyDefauls(obs_data_t* settings);
public:
	void freeBeforeReallocateEffects();
	bool initFilterInGraphicsContext();
	bool initFilterOutsideGraphicsContext();
	//
	void doTick();
	void doRender();
	//

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
	void setEffectParamsChroma(uint32_t swidth, uint32_t sheight);
	void setEffectParamsZoomCrop(uint32_t swidth, uint32_t sheight);
	void setEffectParamsFade(uint32_t swidth, uint32_t sheight);
	void setEffectParamsOutput(TrackedSource* tsourcep, uint32_t swidth, uint32_t sheight, int passNumber, gs_texrender_t* secondaryTexture);
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
	void updateOneShotStatus(bool isStill);
	bool isOneShotEngaged() { return oneShotEngaged; };
	bool isOneShotEngagedAndFirstStage() { return (oneShotEngaged && oneShotStage == 0); };
public:
	void saveVolatileSettings();
	void saveVolatileMarkerZoomScaleSettings(bool isCustomColor);
public:
	void gotoCurrentMarkerlessCoordinates();
public:
	bool updateMarkerlessSettings();
public:
	void viewCycleAdvance(int delta);
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
};
//---------------------------------------------------------------------------



