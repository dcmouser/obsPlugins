#pragma once


//---------------------------------------------------------------------------
#define SETTING_keyMode					"keyMode"
#define TEXT_keyMode					obs_module_text("Key mode")
#define SETTING_Def_keyMode				"chroma"
//
#define SETTING_markerMultiColorMode			"markerMultiColorMode"
#define TEXT_markerMultiColorMode			obs_module_text("Marker Color Style")
#define SETTING_Def_markerMultiColorMode		"color 1"
//---------------------------------------------------------------------------
//
// 
//---------------------------------------------------------------------------
// chroma options based on obs chroma plugin
#define SETTING_CHROMA_COLOR_TYPE1			"key_chroma_color_type1"
#define TEXT_CHROMA_COLOR_TYPE1				obs_module_text("Color 1 type")
#define SETTING_Def_CHROMA_COLOR_TYPE1			"green"
#define SETTING_CHROMA_COLOR1				"key_chroma_color_1"
#define TEXT_CHROMA_COLOR1				obs_module_text("Color 1")
#define SETTING_Def_CHROMA_COLOR1			0x00FF00
#define SETTING_SIMILARITY1				"similarity1"
#define TEXT_SIMILARITY1				obs_module_text("Color 1: Similarity weighting")
#define SETTING_Def_SIMILARITY				365
#define SETTING_SMOOTHNESS1				"smoothness1"
#define TEXT_SMOOTHNESS1				obs_module_text("Color 1: Smoothness weighting")
#define SETTING_Def_SMOOTHNESS				122
//
#define SETTING_CHROMA_COLOR_TYPE2			"key_chroma_color_type2"
#define TEXT_CHROMA_COLOR_TYPE2				obs_module_text("Color 2 type")
#define SETTING_Def_CHROMA_COLOR_TYPE2			"magenta"
#define SETTING_CHROMA_COLOR2				"key_chroma_color_2"
#define TEXT_CHROMA_COLOR2				obs_module_text("Color 2")
#define SETTING_Def_CHROMA_COLOR2			0xFF00FF
#define SETTING_SIMILARITY2				"similarity2"
#define TEXT_SIMILARITY2				obs_module_text("Color 2: Similarity weighting")
#define SETTING_SMOOTHNESS2				"smoothness2"
#define TEXT_SMOOTHNESS2				obs_module_text("Color 2: Smoothness weighting")
//
#define SETTING_testThreshold				"testThreshold"
#define TEXT_testThreshold				obs_module_text("Test threshold (default 500)")
#define SETTING_Def_testThreshold			500
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#define SETTING_HSV_COLOR_TYPE1				"key_hsv_color_type1"
#define TEXT_HSV_COLOR_TYPE1				obs_module_text("Color 1 type")
#define SETTING_Def_HSV_COLOR_TYPE1			"green"
#define SETTING_HSV_COLOR1				"keyhsv_color_1"
#define TEXT_HSV_COLOR1					obs_module_text("Color 1")
#define SETTING_Def_HSV_COLOR1				0x00FF00
//
#define SETTING_hueThreshold1				"hueThreshold1"
#define TEXT_hueThreshold1				obs_module_text("Color 1: Hue Threshold")
#define SETTING_Def_hueThreshold1			100
#define SETTING_saturationThreshold1			"saturationThreshold1"
#define TEXT_saturationThreshold1			obs_module_text("Color 1: Saturation Threshold")
#define SETTING_Def_saturationThreshold1		100
#define SETTING_valueThreshold1				"valueThreshold1"
#define TEXT_valueThreshold1				obs_module_text("Color 1: Value Threshold")
#define SETTING_Def_valueThreshold1			100
//
#define SETTING_HSV_COLOR_TYPE2				"key_hsv_color_type2"
#define TEXT_HSV_COLOR_TYPE2				obs_module_text("Color 2 type")
#define SETTING_Def_HSV_COLOR_TYPE2			"magenta"
#define SETTING_HSV_COLOR2				"keyhsv_color_2"
#define TEXT_HSV_COLOR2					obs_module_text("Color 2")
#define SETTING_Def_HSV_COLOR2				0xFF00FF
//
#define SETTING_hueThreshold2				"hueThreshold2"
#define TEXT_hueThreshold2				obs_module_text("Color 2: Hue Threshold")
#define SETTING_Def_hueThreshold2			100
#define SETTING_saturationThreshold2			"saturationThreshold2"
#define TEXT_saturationThreshold2			obs_module_text("Color 2: Saturation Threshold")
#define SETTING_Def_saturationThreshold2		100
#define SETTING_valueThreshold2				"valueThreshold2"
#define TEXT_valueThreshold2				obs_module_text("Color 2: Value Threshold")
#define SETTING_Def_valueThreshold2			100
//
#define SETTING_hsvLightAdjust1				"hsvLightMod1"
#define TEXT_hsvLightAdjust1				obs_module_text("Color 1: Lighting adjustment")
#define SETTING_Def_hsvLightAdjust1			0
#define SETTING_hsvLightAdjust2				"hsvLightMod2"
#define TEXT_hsvLightAdjust2				obs_module_text("Color 2: Lighting adjustment")
#define SETTING_Def_hsvLightAdjust2			0
//---------------------------------------------------------------------------
//
//
//
//---------------------------------------------------------------------------
// more
#define SETTING_dilateGreen				"dilateGreen"
#define TEXT_dilateGreen				"Dilate steps (color 1)"
#define SETTING_Def_dilateGreen				2
#define SETTING_dilateRed				"dilateRed"
#define TEXT_dilateRed					"Dilate steps (color 2)"
#define SETTING_Def_dilateRed				2
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
#define SETTING_debugRegions				"dbgRegions"
#define TEXT_debugRegions				obs_module_text("Debug overlay region detection (to assist setting marker options).")
#define SETTING_Def_debugRegions			false
#define SETTING_debugChroma				"dbgChroma"
#define TEXT_debugChroma				obs_module_text("Hide background camera image (to assist configuring chroma keying options)")
#define SETTING_Def_debugChroma				false
#define SETTING_debugAllUpdate				"dbgAllUpdate"
#define TEXT_debugAllUpdate				obs_module_text("When showing debug overlay, force all cameras to update every cycle")
#define SETTING_Def_debugAllUpdate			true

#define SETTING_autoTrack				"autoTrack"
#define TEXT_autoTrack					obs_module_text("Auto track marker locations (set rate below)")
#define SETTING_Def_autoTrack				true
#define SETTING_trackRate				"trackRate"
#define TEXT_trackRate					obs_module_text("Scan for marker positions every N frames")
#define SETTING_Def_trackRate				3
//
// valid region filtering
#define SETTING_rmDensityMin				"rmTDensityMin"
#define TEXT_rmDensityMin				obs_module_text("Minimum density (square is 100%, dimaond or circle 50%)")
#define SETTING_Def_rmDensityMin			50
#define SETTING_rmAspectMin				"rmTAspectMin"
#define TEXT_rmAspectMin				obs_module_text("Minimum aspect ratio (square and circle are 100%, rectangle less)")
#define SETTING_Def_rmAspectMin				70
#define SETTING_rmSizeMin				"rmSizeMin"
#define TEXT_rmSizeMin					obs_module_text("Minimum size")
#define SETTING_Def_rmSizeMin				3
#define SETTING_rmSizeMax				"rmSizeMax"
#define TEXT_rmSizeMax					obs_module_text("Maximum size")
#define SETTING_Def_rmSizeMax				100
#define SETTING_rmStageSize				"rmStageSize"
#define TEXT_rmStageSize				obs_module_text("Stage size for doing visual analysis (see help; default 3)")
#define SETTING_Def_rmStageSize				3
#define SETTING_Max_rmStageSize				5
#define SETTING_rmTooCloseDist				"rmTooCloseDist"
#define TEXT_rmTooCloseDist				obs_module_text("Marker separation distance to ignore")
#define SETTING_Def_rmTooCloseDist			60

#define SETTING_rmMinColor2Percent			"rmMinColor2Percent"
#define TEXT_rmMinColor2Percent				obs_module_text("Minimum % of color 2 in dual mode")
#define SETTING_Def_rmMinColor2Percent			5
#define SETTING_rmMaxColor2Percent			"rmMaxColor2Percent"
#define TEXT_rmMaxColor2Percent				obs_module_text("Maximum % of color 2 in dual mode")
#define SETTING_Def_rmMaxColor2Percent			95

// zoomCrop stuff
#define SETTING_zcPreserveAspectRatio			"zcPreserveAspectRatio"
#define TEXT_zcPreserveAspectRatio			obs_module_text("Preserve aspect ratio of display")
#define SETTING_Def_zcPreserveAspectRatio		true

#define SETTING_zcMarkerPos				"zcMarkerPos"
#define TEXT_zcMarkerPos				obs_module_text("Position of view relative to markers")
#define SETTING_Def_zcMarkerPos				"center"

#define SETTING_zcBoxMargin				"zcBoxMargin"
#define TEXT_zcBoxMargin				obs_module_text("Position margin from markers")
#define SETTING_Def_zcBoxMargin				0
#define SETTING_zcBoxMoveSpeed				"zcBoxMoveSpeed"
#define TEXT_zcBoxMoveSpeed				obs_module_text("Travel speed to new marker location")
#define SETTING_Def_zcBoxMoveSpeed			8
#define SETTING_zcBoxMoveDelay				"zcBoxMoveDelay"
#define TEXT_zcBoxMoveDelay				obs_module_text("Delay between traveling to new marker location")
#define SETTING_Def_zcBoxMoveDelay			10

// used for a bunch of things which are derived from it
#define SETTING_zcReactionDistance			"zcReactionDistance"
#define TEXT_zcReactionDistance				obs_module_text("Movement distance considered insignificant")
#define SETTING_Def_zcReactionDistance			35


#define SETTING_zcAlignment				"Alignment"
#define TEXT_zcAlignment				obs_module_text("Alignment when view doesn't fill screen")
#define SETTING_Def_zcAlignment				"center"

#define SETTING_zcMode					"ZcMode"
#define TEXT_zcMode					obs_module_text("Zoom vs. Crop mode")
#define SETTING_Def_zcMode				"zoom and crop"
#define Def_zcMode_ZoomCrop				0
#define Def_zcMode_OnlyCrop				1
#define Def_zcMode_OnlyZoom				2

#define SETTING_zcCropStyle				"ZcCropStyle"
#define TEXT_zcCropStyle				obs_module_text("Crop style")
#define SETTING_Def_zcCropStyle				"black bars"
#define Def_zcCropStyles_blackBars			0
#define Def_zcCropStyle_blur				1

#define SETTING_zcMaxZoom				"MaxZoom"
#define TEXT_zcMaxZoom					obs_module_text("Maximum zoom magnification")
#define SETTING_Def_zcMaxZoom				999
//
#define SETTING_zcOutputSize				"OutputSize"
#define TEXT_zcOutputSize				obs_module_text("Output size (Width x Height); default 1920x1080")
#define SETTING_Def_zcOutputSize			"1920x1080"



// markerless
#define SETTING_markerlessMode				"markerlessMode"
#define TEXT_markerlessMode				obs_module_text("Markerless mode")
#define SETTING_Def_markerlessMode			"manualZoom"
//
#define SETTING_manualViewSourceIndex			"manualViewSourceIndex"
#define TEXT_manualViewSourceIndex			obs_module_text("Current/manually source shown when markerless cycle is disabled")
#define SETTING_DEF_manualViewSourceIndex		0
//
#define SETTING_zcMarkerlessCycleList			"markerlessCycleList"
#define TEXT_zcMarkerlessCycleList			obs_module_text("Markerless cycle list (| separated s=#,z=#.#,a=[ul|uc|ur|ml|mc|mr|ll|lc|lr])")
#define SETTING_Def_zcMarkerlessCycleList		"s=0,z=0 | s=1,z=0 | s=1,z=1,a=mc | s=0,z=2,a=lc"
#define DefMarkerlessCycleListBufMaxSize		255
#define SETTING_markerlessCycleIndex			"markerlessCycleIndex"
#define TEXT_markerlessCycleIndex			obs_module_text("Current markerless cycle entry above")
#define SETTING_Def_markerlessCycleIndex		0

#define SETTING_enableAutoSourceHunting			"enableAutoSourceHunting"
#define TEXT_enableAutoSourceHunting			obs_module_text("Enable automatic source switching (prefer closer zoom = latter source)")
#define SETTING_Def_enableAutoSourceHunting		true

#define SETTING_manualZoomSourceIndex			"manualZoomSourceIndex"
#define TEXT_manualZoomSourceIndex			obs_module_text("Current source id #")
#define SETTING_Def_manualZoomSourceIndex		0
#define SETTING_manualZoomSourceScale			"manualZoomSourceScale"
#define TEXT_manualZoomSourceScale			obs_module_text("Current zoom scale")
#define SETTING_Def_manualZoomSourceScale		100
#define SETTING_manualZoomStepSize			"manualZoomStepSize"
#define TEXT_manualZoomStepSize				obs_module_text("Zoom change per hotkey trigger (percent)")
#define SETTING_Def_manualZoomStepSize			10
#define SETTING_manualZoomAlignment			"manualZoomAlignment"
#define TEXT_manualZoomAlignment			obs_module_text("Zoomed area alignment")
#define SETTING_Def_manualZoomAlignment			"center"
#define SETTING_manualZoomSourceTransitionList		"manualZoomSourceTransitionList"
#define TEXT_manualZoomSourceTransitionList		obs_module_text("Camera source transition zoom levels (see help; comma separate)")
#define SETTING_Def_manualZoomSourceTransitionList	"2"
#define SETTING_manualZoomMinZoom			"manualZoomMinZoom"
#define TEXT_manualZoomMinZoom				obs_module_text("Minimum zoom level")
#define SETTING_Def_manualZoomMinZoom			50




#define SETTING_zcEasing				"zcEasing"
#define TEXT_zcEasing					obs_module_text("Travel easing")
#define SETTING_Def_zcEasing				"eased"
//
#define SETTING_srcN					"NumSources"
#define TEXT_srcN					obs_module_text("Number of top-down camera sources")
#define SETTING_Def_srcN				1

#define SETTING_fadeMode				"fadeMode"
#define TEXT_fadeMode					obs_module_text("Fade mode when switching between camera sources")
#define SETTING_Def_fadeMode				"normal"

// for our list of sources
#define SETTING_sourceNameWithArg			"src%d"
#define TEXT_sourceNameWithArg				obs_module_text("Source %d")
#define SETTING_sourceZoomScaleWithArg			"srcZs%d"
#define TEXT_sourceZoomScaleWithArg			obs_module_text("Source %d zoom scale modifier (default 100; see help)")

//
#define SETTING_missingMarkerPulloutTimeout		"zcMissingMarkerPulloutTimeout"
#define TEXT_missingMarkerPulloutTimeout		obs_module_text("Missing marker timeout (default 40)")
#define SETTING_Def_missingMarkerPulloutTimeout		30
//
#define SETTING_validMarkersToCheckForOcclusion		"zcValidMarkersCheckOcclusion"
#define TEXT_validMarkersToCheckForOcclusion		obs_module_text("Valid markers to check for occlusion (default 5)")
#define SETTING_Def_validMarkersToCheckForOcclusion	5
//
#define SETTING_blurPasses				"zcBlurPasses"
#define TEXT_blurPasses					obs_module_text("Blur: Number of passes (default 6)")
#define SETTING_Def_blurPasses				6
#define SETTING_blurSizeReduce				"zcBlurSizeReduce"
#define TEXT_blurSizeReduce				obs_module_text("Blur: Size reduce (default 12)")
#define SETTING_Def_blurSizeReduce			12
#define SETTING_blurExteriorDull			"zcBlur"
#define TEXT_blurExteriorDull				obs_module_text("Blur: Exterior dullness (default 5)")
#define SETTING_Def_blurExteriorDull			5
//
#define SETTING_avoidTrackingDuringTransition		"zcAvoidTrackingInTransitions"
#define TEXT_avoidTrackingDuringTransition		obs_module_text("Avoid tracking during transitions (broken)")
#define SETTING_Def_avoidTrackingDuringTransition	false
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// non-confiruable options

// try to speed up render if we are just rendering the exact source as it no zoom/crop/etc.
#define DefBypassZoomOnUntouchedOutput			false
// for the showImage effect option alpha value
#define DefEffectAlphaShowImage				0.25f
// momentum thresholds
#define DefMomentumCounterTargetNormal			5
#define DefMomentumCounterTargetMissingMarkers		15
//

#define DefChangeMomentumDistanceThresholdMoving	((float)opt_zcReactionDistance/2.0f)
#define DefChangeMomentumDistanceThresholdStabilized	((float)opt_zcReactionDistance/1.5f)
//
#define DefThresholdTargetStableDistance		((float)opt_zcReactionDistance)
//
#define DefChangeMomentumDistanceThresholdDontDrift	((float)opt_zcReactionDistance/4.0f)
#define DefIntegerizeStableAveragingLocation		false
//
#define DefThresholdTargetStableCount			((float)opt_zcBoxMoveDelay/2.0f)
//
#define DefAverageSmoothingRateToCloseTarget		0.90f
//
#define DefMaxSources					10
#define DefNameLenSource				80
//
#define DefAlwaysUpdateTrackingWhenHunting		true

// ATTN: 12/8/22 setting this to true can degrate performance
#define DefAlwaysUpdateTrackingWhenOneShotting		false


// set this to low value to fix failure to zoom in; set long to try to find insane obs bug failure of source to update
#define DefSettledLookingPositionRecheckLimit		40

#define DefForSingleValidRegionAssumePrevious		true
#define DefMarkerlessStreakCyclesBeforeSwitch		2

// set to 500 for half a second of tweaking of zoomcrop to markers after a oneshot settles
#define DEF_MsSecsToHuntAfterOneShotSettles		500

#define DefMaxMarkerlessEntries				20
#define DefFadeDuration					0.5f;

// setting to 1 means fades are rendered at full resolution, 2 or more reduces sizes of textures during fade; but it doesnt seem to have noticable cpu effect so leave at 1
#define DefFadeTextureReduction				1

#define DefDelayHuntingBriefTimeMs			500

#define SavedMarkerCheckSameDistance			100
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// attempt to fix a weird problem where top down (4k) camera seems to uncache if it doesn't get used in a while (5min) and thus misses frames
// i currently kludge this using a source dock to ALWAYS show a tiny dock of the 4k top down camera on screen, which seems extremely wasteful of cpu and desktop space
// but i'd like to find out a way to kludge a hidden refresh of it every so often
//
// this works and is needed (not sure if rate needs to be so high); this affects when we are actually rendering our view
// ATTN: 3/30/23
//#define DefKludgeTouchAllSourcesOnRenderCycle		true
#define DefKludgeTouchAllSourcesOnRenderCycle		false
#define DefKludgeTouchAllSourcesOnRenderCycleRate	2
//
// new 2/25/23 -- new attempt to keep it cached even in tick() cycle - this CRASHES if set true, AND doesnt seem to help??
// ATTN: 3/30/23
//#define DefKludgeTouchAllSourcesOnNonRenderCycle	true
#define DefKludgeTouchAllSourcesOnNonRenderCycle	false
#define DefKludgeTouchAllSourcesOnNonRenderCycleRate	2
//
// experimental:
#define DefToggleChildSourcesHiddenShow			true
//---------------------------------------------------------------------------

 
//---------------------------------------------------------------------------
// i made a change in the algorithm to fix what appeared to be a bug; now coming back weeks later it seems the original code was good?
#define DefUseJesseCounterTraceFix			false
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// this hides audio of source
// we dont use these

#define DefDebugClearDrawingSpaced			false
#define DefDebugObsGraphicsKludge			false
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// ones we use long term:
#define DefDebugDisableChromaCalibrate			true
#define	DefDebugComplainBadIndexInRegionDetector	false
// trying to change from true to false on 2/25/23, doesnt seem to make any difference
#define DefDebugDontEnumerateSourceOnRequest		true
//#define DefDebugDontEnumerateSourceOnRequest		false
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// testing
// just to help us do temporary quick debug checks
#define DefDebugTest					false
#define DefDebugTestSkipKludgeRefreshTouchRendering	false
#define DefDebugTryReallocateStagingSurfaceEveryTime	false
#define DefDebugDoForegroundPixelCountTest		false
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// hotkey signals that allow us to send messages to fellow AutoZoomSources
#define DefActionSignalStructPreset_keyUp		true
#define DefActionSignalStructPreset_modifiers		1234
#define DefActionSignalStructPreset_native_modifiers	1234
//#define DefActionSignalStructPreset_native_scancode	1234
#define DefActionSignalStructPreset_native_vkey		1234
//
#define DefActionSignalKeyToggleAutoUpdate		'a'
#define DefActionSignalKeyToggleLastGoodMarkers		'b'
#define DefActionSignalKeyCycleForward			'c'
#define DefActionSignalKeyCycleBackward			'd'
#define DefActionSignalKeyToggleAutoSourceHunting	'e'
#define DefActionSignalKeyInitiateOneShot		'f'
#define DefActionSignalKeyToggleCropping		'g'
#define DefActionSignalKeyToggleDebugDisplay		'h'
#define DefActionSignalKeyToggleCropBlurMode		'i'
#define DefActionSignalKeyResetView			'j'

//---------------------------------------------------------------------------
