//---------------------------------------------------------------------------
#include "jrPlugin.h"
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrPlugin::doTick() {
	// ATTN: there is code in render() that could be placed here..
	// ATTN: rebuild sources from multisource effect plugin code -- needed every tick?

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
	return stracker.parseSettingString(opt_markerlessCycleListBuf);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
bool JrPlugin::initFilterInGraphicsContext() {
	// called once at creation startup

	// chroma effect
	char *effectChroma_path = obs_module_file("ObsAutoZoomChroma.effect");
	effectChroma = gs_effect_create_from_file(effectChroma_path, NULL);
	if (effectChroma) {
		// effect .effect file uniform parameters
		chroma1_param = gs_effect_get_param_by_name(effectChroma, "chroma_key1");
		similarity1_param = gs_effect_get_param_by_name(effectChroma, "similarity1");
		smoothness1_param = gs_effect_get_param_by_name(effectChroma, "smoothness1");
		chroma2_param = gs_effect_get_param_by_name(effectChroma, "chroma_key2");
		similarity2_param = gs_effect_get_param_by_name(effectChroma, "similarity2");
		smoothness2_param = gs_effect_get_param_by_name(effectChroma, "smoothness2");
		//
		chroma_pixel_size_param = gs_effect_get_param_by_name(effectChroma, "pixel_size");
		chromaThreshold_param = gs_effect_get_param_by_name(effectChroma, "threshold");
	}
	bfree(effectChroma_path);

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

	// helper texrenders
	outTexRenderBase = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	outTexRenderA = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	outTexRenderB = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	fadeTexRenderA = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	fadeTexRenderB = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	// success?
	if (!effectChroma || !effectZoomCrop || !effectFade) {
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
	hotkeyId_CycleSourceBack = hotkeyId_CycleViewForward = hotkeyId_CycleViewBack = hotkeyId_toggleEnableMarkerlessUse = hotkeyId_toggleAutoSourceHunting = -1;

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
	if (effectChroma) {
		gs_effect_destroy(effectChroma);
		effectChroma = NULL;
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
	//mydebug("In updateComputedOptions.");
	computedChangeMomentumDistanceThresholdMoving = DefChangeMomentumDistanceThresholdMoving;
	computedChangeMomentumDistanceThresholdStabilized = DefChangeMomentumDistanceThresholdStabilized;
	computedThresholdTargetStableDistance = DefThresholdTargetStableDistance;
	computedThresholdTargetStableCount = (int)DefThresholdTargetStableCount;
	computedChangeMomentumDistanceThresholdDontDrift = DefChangeMomentumDistanceThresholdDontDrift;
	computedAverageSmoothingRateToCloseTarget = DefAverageSmoothingRateToCloseTarget;
	computedIntegerizeStableAveragingLocation = DefIntegerizeStableAveragingLocation;
	computedMomentumCounterTargetMissingMarkers = DefMomentumCounterTargetMissingMarkers;
	computedMomentumCounterTargetNormal = DefMomentumCounterTargetNormal;
	//computedMinDistBetweenMarkersIsTooClose = DefMinDistBetweenMarkersIsTooClose;
	//computedMinDistBetweenMarkersIsTooClose = opt_rmSizeMax * DefMaxSizeToMarkerTooCloseScale;
	computedMinDistBetweenMarkersIsTooClose = (float)opt_rmTooCloseDist;
}
//---------------------------------------------------------------------------

























void JrPlugin::handleHotkeyPress(obs_hotkey_id id, obs_hotkey_t *key) {
	//
	WaitForSingleObject(mutex, INFINITE);
	//
	// trigger hotkey
	if (id == hotkeyId_ToggleAutoUpdate) {
		opt_enableAutoUpdate = !opt_enableAutoUpdate;
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
		saveCurrentViewedSourceAsManualViewOption();
		opt_ignoreMarkers = !opt_ignoreMarkers;
		gotoCurrentMarkerlessCoordinates();
		saveVolatileSettings();
	} else if (id == hotkeyId_CycleViewForward) {
		viewCycleAdvance(1);
		saveVolatileSettings();
	} else if (id == hotkeyId_CycleViewBack) {
		viewCycleAdvance(-1);
		saveVolatileSettings();
	} else if (id == hotkeyId_toggleEnableMarkerlessUse) {
		opt_enableMarkerlessUse = !opt_enableMarkerlessUse;
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
