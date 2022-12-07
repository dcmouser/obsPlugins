//---------------------------------------------------------------------------
#include "jrPlugin.h"
#include "jrazcolorhelpers.h"
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrPlugin::doTick() {
	// ATTN: there is code in render() that could be placed here..
	// ATTN: rebuild sources from multisource effect plugin code -- needed every tick?

	// update this here in doTick to be safe
	if (opt_avoidTrackingInTransitions) {
		currentlyTransitioning = checkIsTransitioning();
	} else {
		currentlyTransitioning = false;
	}

	//
	stracker.checkAndUpdateAllTrackingSources();
	if (sourcesHaveChanged) {
		onSourcesHaveChanged();
	}
}


void JrPlugin::onSourcesHaveChanged() {
	 updateOutputSize();
	sourcesHaveChanged = false;
}
//---------------------------------------------------------------------------






















//---------------------------------------------------------------------------
void JrPlugin::forceUpdatePluginSettingsOnOptionChange() {
	// tell any sources they may need internal updating

	stracker.onOptionsChange();

	if (true) {
		// we may need to updat this even if we bypass the rebuilding cycle
		// custom outputsize and coords -- used even outside render cycle or on bypass
		updateOutputSize();
		updateMarkerlessSettings();
	}

	// any computed options we need to recompute
	updateComputedOptions();

	if (opt_filterBypass || opt_ignoreMarkers) {
		// in bypass mode, we force the view source index when we have options change
		if (opt_manualViewSourceIndex != stracker.getViewSourceIndex() && opt_manualViewSourceIndex!=-1) {
			// force switch of view
			stracker.setViewSourceIndex(opt_manualViewSourceIndex, false);
		}
		if (opt_ignoreMarkers) {
			stracker.travelToMarkerless(stracker.getViewSourceIndex(), true, false);
		}
	} else {
		// in case markerless settings have changed
		stracker.reTravelToMarkerlessIfMarkerless(false);
	}
}


void JrPlugin::updateOutputSize() {
	// recompute output size of US

	updateComputeAutomaticOutputSize();

	int dummyval;
	parseTextCordsString(opt_OutputSizeBuf, &outputWidthPlugin, &outputHeightPlugin, &dummyval,&dummyval,outputWidthAutomatic,outputHeightAutomatic,0,0,outputWidthAutomatic+1,outputHeightAutomatic+1);
	//mydebug("in updateOutputSize with %d x %d and outsize = %dx%d.", outputWidthAutomatic, outputHeightAutomatic, outputWidthPlugin, outputHeightPlugin);
}


bool JrPlugin::updateComputeAutomaticOutputSize() {
	int maxw, maxh;
	stracker.calculateMaxSourceDimensions(maxw, maxh);
	//mydebug("Automatic out dimensions are %d,%d.", maxw, maxh);
	if (maxw == 0 || maxh == 0) {
		// defaults
		maxw = 1920;
		maxh = 1080;
	}
	if (maxw != outputWidthAutomatic || maxh != outputHeightAutomatic) {
		// changes
		outputWidthAutomatic = maxw;
		outputHeightAutomatic = maxh;
		return true;
	}
	// no change
	return false;
}

bool JrPlugin::updateMarkerlessSettings() {

	// manual zoom markerless
	doManualZoomInOutByPercent(0.0);
	// normal markerless
	return stracker.parseSettingString(opt_markerlessCycleListBuf);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
bool JrPlugin::initFilterInGraphicsContext() {
	// called once at creation startup

	// chroma key effect
	char *effectChromaKey_path = obs_module_file("ObsAutoZoomChromaKey.effect");
	effectChromaKey = gs_effect_create_from_file(effectChromaKey_path, NULL);
	if (effectChromaKey) {
		// effect .effect file uniform parameters
		chroma_pixel_size_param = gs_effect_get_param_by_name(effectChromaKey, "pixel_size");
		chroma1_param = gs_effect_get_param_by_name(effectChromaKey, "chroma_key1");
		similarity1_param = gs_effect_get_param_by_name(effectChromaKey, "similarity1");
		smoothness1_param = gs_effect_get_param_by_name(effectChromaKey, "smoothness1");
		chroma2_param = gs_effect_get_param_by_name(effectChromaKey, "chroma_key2");
		similarity2_param = gs_effect_get_param_by_name(effectChromaKey, "similarity2");
		smoothness2_param = gs_effect_get_param_by_name(effectChromaKey, "smoothness2");
		testThreshold_param = gs_effect_get_param_by_name(effectChromaKey, "testThreshold");
	}
	bfree(effectChromaKey_path);

	// hsv key effect
	char *effectHsvKey_path = obs_module_file("ObsAutoZoomHsvKey.effect");
	effectHsvKey = gs_effect_create_from_file(effectHsvKey_path, NULL);
	if (effectHsvKey) {
		// effect .effect file uniform parameters
		hsv_pixel_size_param = gs_effect_get_param_by_name(effectHsvKey, "pixel_size");
		//
		hueThreshold1_param = gs_effect_get_param_by_name(effectHsvKey, "hueThreshold1");
		saturationThreshold1_param = gs_effect_get_param_by_name(effectHsvKey, "saturationThreshold1");
		valueThreshold1_param = gs_effect_get_param_by_name(effectHsvKey, "valueThreshold1");
		hueThreshold2_param = gs_effect_get_param_by_name(effectHsvKey, "hueThreshold2");
		saturationThreshold2_param = gs_effect_get_param_by_name(effectHsvKey, "saturationThreshold2");
		valueThreshold2_param = gs_effect_get_param_by_name(effectHsvKey, "valueThreshold2");
		color1hsv_param = gs_effect_get_param_by_name(effectHsvKey, "color1hsv");
		color2hsv_param = gs_effect_get_param_by_name(effectHsvKey, "color2hsv");
	}
	bfree(effectHsvKey_path);

	// zoomCrop effect
	char *effectZoomCrop_path = obs_module_file("ObsAutoZoomCrop.effect");
	effectZoomCrop = gs_effect_create_from_file(effectZoomCrop_path, NULL);
	if (effectZoomCrop) {
		// effect .effect file uniform parameters
		param_zoom_mul = gs_effect_get_param_by_name(effectZoomCrop, "mulVal");
		param_zoom_add = gs_effect_get_param_by_name(effectZoomCrop, "addVal");
		param_zoom_clip_ul = gs_effect_get_param_by_name(effectZoomCrop, "clip_ul");
		param_zoom_clip_lr = gs_effect_get_param_by_name(effectZoomCrop, "clip_lr");
		param_zoom_hardClip_ul = gs_effect_get_param_by_name(effectZoomCrop, "hardClip_ul");
		param_zoom_hardClip_lr = gs_effect_get_param_by_name(effectZoomCrop, "hardClip_lr");
		param_zoom_pixel_size = gs_effect_get_param_by_name(effectZoomCrop, "pixel_size");
	}
	bfree(effectZoomCrop_path);

	// output effects
	char *effectOutput_path = obs_module_file("ObsAutoZoomOutput.effect");
	effectOutput = gs_effect_create_from_file(effectOutput_path, NULL);
	if (effectOutput) {
		// effect .effect file uniform parameters
		param_output_clip_ul = gs_effect_get_param_by_name(effectOutput, "clip_ul");
		param_output_clip_lr = gs_effect_get_param_by_name(effectOutput, "clip_lr");
		param_output_hardClip_ul = gs_effect_get_param_by_name(effectOutput, "hardClip_ul");
		param_output_hardClip_lr = gs_effect_get_param_by_name(effectOutput, "hardClip_lr");
		param_output_pixel_size = gs_effect_get_param_by_name(effectOutput, "pixel_size");
		param_output_effectInside = gs_effect_get_param_by_name(effectOutput, "effectInside");
		param_output_passNumber = gs_effect_get_param_by_name(effectOutput, "passNumber");
		param_ouput_secondaryTex = gs_effect_get_param_by_name(effectOutput, "secondaryTexture");
		param_ouput_exteriorDullness = gs_effect_get_param_by_name(effectOutput, "exteriorDullness");
	}
	bfree(effectOutput_path);

	// fade effect
	char *effectFade_path = obs_module_file("ObsAutoZoomFade.effect");
	effectFade = gs_effect_create_from_file(effectFade_path, NULL);
	if (effectFade) {
		// effect .effect file uniform parameters - we get these each time currently
		param_fade_a = gs_effect_get_param_by_name(effectFade, "tex_a");
		param_fade_b = gs_effect_get_param_by_name(effectFade, "tex_b");
		param_fade_val = gs_effect_get_param_by_name(effectFade, "fade_val");
	}
	bfree(effectFade_path);

	// dilate effect
	char *effectDilate_path = obs_module_file("ObsAutoZoomDilate.effect");
	effectDilate = gs_effect_create_from_file(effectDilate_path, NULL);
	if (effectDilate) {
		// effect .effect file uniform parameters - we get these each time currently
		param_dilate_pixel_size = gs_effect_get_param_by_name(effectDilate, "pixel_size");
		param_dilateColorFromRgba = gs_effect_get_param_by_name(effectDilate, "dilateColorFromRgba");
		param_dilateColorToRgba = gs_effect_get_param_by_name(effectDilate, "dilateColorToRgba");
		param_dilateBackgroundRgba = gs_effect_get_param_by_name(effectDilate, "dilateBackgroundRgba");
	}
	bfree(effectDilate_path);


	// helper texrenders
	outTexRenderBase = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	outTexRenderA = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	outTexRenderB = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	fadeTexRenderA = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	fadeTexRenderB = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	// Dilation vector constants, etc.
	jrazFillRgbaVec(colorGreenAsRgbaVec, 0.0, 1.0, 0.0, 1.0);
	jrazFillRgbaVec(colorRedAsRgbaVec, 1.0, 0.0, 0.0, 1.0);
	jrazFillRgbaVec(colorBackgroundAsRgbaVec, 0.0, 0.0, 0.0, 0.0);
	jrazFillRgbaVec(colorGreenTempAsRgbaVec, 0.0, 0.0, 1.0, 1.0);
	jrazFillRgbaVec(colorRedTempAsRgbaVec, 0.0, 1.0, 1.0, 1.0);

	// success?
	if (!effectChromaKey || !effectHsvKey || !effectZoomCrop || !effectFade || !effectDilate) {
		return false;
	}

	// success
	return true;
}





bool JrPlugin::initFilterOutsideGraphicsContext() {
	// called once at creation startup
	in_enumSources = false;
	//
	firstRender = true;
	sourcesHaveChanged = false;
	kludgeTouchCounter = 0;
	trackingUpdateCounter = 0;
	sourceDetailsHaveChanged = false;
	opt_markerlessCycleIndex = 0;
	forceAllAnalyzeMarkersOnNextRender = false;

	// hotkeys init
	hotkeyId_ToggleAutoUpdate = hotkeyId_OneShotZoomCrop = hotkeyId_ToggleCropping = hotkeyId_ToggleDebugDisplay = hotkeyId_ToggleIgnoreMarkers = hotkeyId_CycleSource = -1;
	hotkeyId_CycleSourceBack = hotkeyId_CycleViewForward = hotkeyId_CycleViewBack = hotkeyId_toggleAutoSourceHunting = -1;

	//
	mutex = CreateMutexA(NULL, FALSE, NULL);

	// reset
	cancelFade();
	cancelOneShot();

	// clear sources
	stracker.init(this);

	//mydebug("Back from calling stracker init.");

	// success
	return true;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrPlugin::freeBeforeReallocateEffects() {
	if (effectChromaKey) {
		gs_effect_destroy(effectChromaKey);
		effectChromaKey = NULL;
	}
	if (effectHsvKey) {
		gs_effect_destroy(effectHsvKey);
		effectHsvKey = NULL;
	}
	if (effectZoomCrop) {
		gs_effect_destroy(effectZoomCrop);
		effectZoomCrop = NULL;
	}
	if (effectOutput) {
		gs_effect_destroy(effectOutput);
		effectOutput = NULL;
	}
	if (effectFade) {
		gs_effect_destroy(effectFade);
		effectFade = NULL;
	}
	if (effectDilate) {
		gs_effect_destroy(effectDilate);
		effectDilate = NULL;
	}
	if (outTexRenderBase != NULL) {
		gs_texrender_destroy(outTexRenderBase);
		outTexRenderBase = NULL;
	}
	if (outTexRenderA != NULL) {
		gs_texrender_destroy(outTexRenderA);
		outTexRenderA = NULL;
	}
	if (outTexRenderB != NULL) {
		gs_texrender_destroy(outTexRenderB);
		outTexRenderB = NULL;
	}
	if (fadeTexRenderA != NULL) {
		gs_texrender_destroy(fadeTexRenderA);
		fadeTexRenderA = NULL;
	}
	if (fadeTexRenderB != NULL) {
		gs_texrender_destroy(fadeTexRenderB);
		fadeTexRenderB = NULL;
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::adjustToSourceSizeChanges(bool flagForce) {
	if (!sourcesHaveChanged && !flagForce) {
		// nothing to do
		return;
	}
	// ok one or more sources have changed sizes, which may trigger us to change our STAGING size?

	// clear flag
	sourcesHaveChanged = false;
}


void JrPlugin::convertStageSizeIndexToDimensions(int stageSizeIndex, int& width, int& height) {
	// go from user option for stage size index to dimensions
	const int widths[] = { 64,120,240,480,960,1920, 3840 };
	float aspectRatio = 1080.0f / 1920.0f;
	if (outputWidthAutomatic > 0) {
		aspectRatio = (float)outputHeightAutomatic / (float)outputWidthAutomatic;
	}
	// ATTN: TODO compute actual aspect ratio from max source size?
	stageSizeIndex = max(min(stageSizeIndex, 6), 0);
	width = widths[stageSizeIndex];
	height = (int)(width * aspectRatio);
}



void JrPlugin::computeStageSizeMaxed(int& width, int& height, int maxWidth, int maxHeight) {
	convertStageSizeIndexToDimensions(opt_rmStageSize, width, height);
	if (width > maxWidth || height > maxHeight) {
		width = maxWidth;
		height = maxHeight;
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrPlugin::updateComputedOptions() {
	// ATTN: TO DO - change some of these counters to be based on update rate
	// ATTN: and do the distance ones need to scale based on 4k res?
	// see jrPluginDefs.h for contstants
	//
	float distScale = 1.0f;
	//float counterScale = 1.0f;
	// test
	float counterScale = 2.0f / (float)opt_updateRate;
	// 
	//mydebug("In updateComputedOptions.");
	computedChangeMomentumDistanceThresholdMoving = DefChangeMomentumDistanceThresholdMoving * distScale;
	computedChangeMomentumDistanceThresholdStabilized = DefChangeMomentumDistanceThresholdStabilized * distScale;
	computedThresholdTargetStableDistance = DefThresholdTargetStableDistance * distScale;
	computedChangeMomentumDistanceThresholdDontDrift = DefChangeMomentumDistanceThresholdDontDrift * distScale;
	computedMinDistBetweenMarkersIsTooClose = (float)opt_rmTooCloseDist * distScale;
	//
	computedAverageSmoothingRateToCloseTarget = DefAverageSmoothingRateToCloseTarget;
	computedIntegerizeStableAveragingLocation = DefIntegerizeStableAveragingLocation;
	//
	// ATTN: should these scale with update rate, OR be based on clock time
	computedThresholdTargetStableCount = (int)((float)DefThresholdTargetStableCount * counterScale);
	computedMomentumCounterTargetMissingMarkers = (int)((float)DefMomentumCounterTargetMissingMarkers * counterScale);
	computedMomentumCounterTargetNormal = (int)((float)DefMomentumCounterTargetNormal * counterScale);
	//

}
//---------------------------------------------------------------------------




















// 11/5/22
// a word on opt_enableAutoUpdate vs opt_ignoreMarkers, since they are so similar
// when enabled, opt_ignoreMarkers will instantly switch to a preconfigured default markerless view, as if no markers were present
// when disabled, opt_enableAutoUpdate will stop seeking markers and changing to adapte to them, but will NOT move from their current position if they had last adjusted to a marker
// so you can adjust to a marker and then disable opt_enableAutoUpdate to hold position.
// But it is confusing to have both, and need hotkey for both.
// To unify, we could have a hotkey for toggling opt_enableAutoUpdate and then a hotkey for forcing opt_enableAutoUpdate off AND reseting view to preconfigured default markerless view once
// That is, "ignore markerless" is not reall needed, it is a combination of turning off auto update and also resetting view
// So this would still need 2 hotkeys BUT not two TOGGLE hotkeys, which is confusing.
// This makes more sense because some combinations of the toggles make no sense and some are redundant -- it doesnt ever make sense to have ignore on and update on, and if ignore is on, state of update is irrelevant
// PROBLEM: The only loss with this scheme is that with two toggles one could use markers to designate a zoom region, and then turn off auto update, and then easily toggle between using that zoom region vs default markerless, even without markers,
// assuming the previous marker position remains unchanged.  If we do away with the two toggles, then the only way to switch away from the remembered marker positions is to clear and forget them.




void JrPlugin::handleHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
	//
	WaitForSingleObject(mutex, INFINITE);
	//
	// trigger hotkey
	if (id == hotkeyId_ToggleAutoUpdate) {
		// ATTN: TODO - should we combine ignore markers with auto update and just have one setting?
		opt_enableAutoUpdate = !opt_enableAutoUpdate;
		if (opt_enableAutoUpdate) {
			// if we turn on auto update, then we turn off ignore markers
			opt_ignoreMarkers = false;
		}
		saveVolatileSettings();
	} else if (id == hotkeyId_OneShotZoomCrop) {
		initiateOneShot();
	} else if (id == hotkeyId_ToggleCropping) {
		if (opt_zcMode == Def_zcMode_OnlyZoom) {
			opt_zcMode = Def_zcMode_ZoomCrop;
		} else {
			opt_zcMode = Def_zcMode_OnlyZoom;
		}
		saveVolatileSettings();
	} else if (id == hotkeyId_ToggleDebugDisplay) {
		opt_debugRegions = !opt_debugRegions;
		saveVolatileSettings();
	} else if (id == hotkeyId_ToggleIgnoreMarkers) {
		// ATTN: TODO - should we combine ignore markers with auto update and just have one setting?
		saveCurrentViewedSourceAsManualViewOption();
		opt_ignoreMarkers = !opt_ignoreMarkers;
		if (!opt_ignoreMarkers) {
			// if we turn off ignore markers, we turn on auto update
			opt_enableAutoUpdate = true;
		}
		gotoCurrentMarkerlessCoordinates();
		saveVolatileSettings();
	} else if (id == hotkeyId_CycleViewForward) {
		// ATTN: TODO - should we force ignore markers in this case?
		viewCycleAdvance(1);
		saveVolatileSettings();
	} else if (id == hotkeyId_CycleViewBack) {
		// ATTN: TODO - should we force ignore markers in this case?
		viewCycleAdvance(-1);
		saveVolatileSettings();
	} else if (id == hotkeyId_toggleAutoSourceHunting) {
		saveCurrentViewedSourceAsManualViewOption();
		opt_enableAutoSourceHunting = !opt_enableAutoSourceHunting;
		saveVolatileSettings();
	} else {
		// unknown hotkey
		/*
		if (id == hotkeyId_ToggleBypass) {
			saveCurrentViewedSourceAsManualViewOption();
			opt_filterBypass = !opt_filterBypass;
			saveVolatileSettings();
		}
		*/ 
	}
	//
	ReleaseMutex(mutex);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
bool JrPlugin::checkIsTransitioning() {
	// this code hangs obs for reasons unknown to me
	bool isTransitioning = false;
	obs_source_t* transition = obs_get_output_source(0);
	if (transition) {
		float t = obs_transition_get_time(transition);
		isTransitioning = t < 1.0f && t > 0.0f;
		if (isTransitioning) {
			// blog(LOG_WARNING, "skipping autozoom tracking update because in the middle of a transition.");
		}
		obs_source_release(transition);
	}
	return isTransitioning;
}
//---------------------------------------------------------------------------
