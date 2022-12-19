//---------------------------------------------------------------------------
#include <cstdio>
//
#include "jrPlugin.h"
#include "jrazcolorhelpers.h"
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrPlugin::doRender() {
	uint32_t sourceViewWidth, sourceViewHeight;
	uint32_t sourceTrackWidth, sourceTrackHeight;
	bool didUpdateTrackingBox = false;
	bool isHunting = false;
	bool validSources = true;


	// adjust to any changed source sizes; this may cause stage sizes and graphics buffers to be reallocated, etc. IF needed
	adjustToSourceSizeChanges(false);


	// in filter mode, force first source details that are only valid during this function
	if (isPluginTypeFilter()) {
		// ok here we force the source pointer for source 0, we will want to clear this before we leave
		// this will tell trackedSource0 to return this pointer when asked to borrowAFullSource and do nothing on freeof it
		stracker.setExternallyManagedTrackedSource(0, obs_filter_get_target(context));
	}



	// sort of kludge for a quick forced update tracking of all sources, which the options may need
	if (forceAllAnalyzeMarkersOnNextRender) {
		// clear flag
		forceAllAnalyzeMarkersOnNextRender = false;
		int sourceCount = stracker.getSourceCount();
		for (int i = 0; i < sourceCount; ++i) {
			TrackedSource* tsp = stracker.getTrackedSourceByIndex(i);
			if (true) {
				// rent source
				obs_source_t* sp = tsp->borrowFullSourceFromWeakSource();
				if (sp) {
					// tell source to upgrad markers
					tsp->findTrackingMarkerRegionInSource(sp, true, false, true);
					//tsp->updateZoomCropBoxFromCurrentCandidate(true);
					// free source
					tsp->releaseBorrowedFullSource(sp);
				}
			}
		}
	}




	// get the TrackedSources for viewing and tracking
	// if they are the same (ie we aren't currently hunting for a new source(camera)) then tracking source will be NULL, meaning use view source
	TrackedSource* tsourcepView = NULL;
	TrackedSource* tsourcepTrack = NULL;
	obs_source_t* sourceView = NULL;
	obs_source_t* sourceTrack = NULL;

	stracker.getDualTrackedViews(tsourcepView, tsourcepTrack);

	// now full sources from the weak references -- remember these need to be released before we leave this function
	sourceView = tsourcepView ? tsourcepView->borrowFullSourceFromWeakSource() : NULL;
	sourceTrack = tsourcepTrack ? tsourcepTrack->borrowFullSourceFromWeakSource() : NULL;

	// do a quick size check on each source to recreate its memory if it changes sizes or on startuo
	// ATTN: TODO see if we should do this in tick() or can automatically do it more efficiently only occasionally

	if (tsourcepView) {
		if (true || tsourcepView->needsRecheckForSize) {
			validSources &= tsourcepView->recheckSizeAndAdjustIfNeeded(sourceView);
		}
	}
	if (tsourcepTrack) {
		if (true || tsourcepTrack->needsRecheckForSize) {
			validSources &= tsourcepTrack->recheckSizeAndAdjustIfNeeded(sourceTrack);
		}
	}

	if (sourceView == NULL || tsourcepView->getSourceWidth() == 0) {
		// this can happen at startup
		//mydebug("actual sources no good.");
		validSources = false;
	}

	// we might mark sources as invalid on our first render to give us time to go to initial view
	if (validSources && firstRender) {
		// skip this first valid render and set initial view instead
		firstRender = false;
		validSources = false;
		goToInitialView();
	}


	// first place we might check for transitioning -- we do not use this
	if (false && validSources && opt_avoidTrackingInTransitions) {
		// new code to try to avoid tracking if in middle of transition
		if (currentlyTransitioning) {
			validSources = false;
		}
	}


	// sources good to use, if not we skill all the work
	if (validSources) {

		// are we doing tracking? either at a certain rate or during oneshots or when hunting
		bool shouldUpdateTrackingBox = false;

		// next place we could check for transitioning
		if (true && opt_avoidTrackingInTransitions && currentlyTransitioning) {
			// skip updating
		} else if (stracker.isTrackingDelayed()) {
			// if delayed tracking
		} else if (tsourcepTrack && DefAlwaysUpdateTrackingWhenHunting) {
			// when we are hunting we always update tracking
			trackingUpdateCounter = 0;
			shouldUpdateTrackingBox = true;
		} else if (opt_autoTrack || oneShotEngaged || tsourcepTrack) {
			if (++trackingUpdateCounter >= opt_trackRate || (oneShotEngaged && (DefAlwaysUpdateTrackingWhenOneShotting || !oneShotDidAtLeastOneTrack))) {
				// frequence to update tracking box
				trackingUpdateCounter = 0;
				shouldUpdateTrackingBox = true;
			}
		}


		// default source to track
		obs_source_t* sourcePointerToTrack = tsourcepTrack ? sourceTrack : sourceView;
		TrackedSource* trackedSourcePointerToTrack = tsourcepTrack ? tsourcepTrack : tsourcepView;
		int trackedIndexToTrack = tsourcepTrack ? stracker.getTrackSourceIndex() : stracker.getViewSourceIndex();


		// record the hunting index - this is an ugly consequence of advancing the hunt before the end of this function
		//mydebug("In dorender stage 1b.");
		if (tsourcepTrack) {
			// we are hunting, record the index that we are currently checking, BEFORE we advance
			isHunting = true;
		}



		bool needsRendering = false;
		//mydebug("after possible advance we have isHunting = %d and viewindex = %d and trackindex = %d and trackedIndexToTrack = %d.", (int)isHunting, stracker.sourceIndexViewing, stracker.sourceIndexTracking, trackedIndexToTrack);


		// OVERRIDE display
		if (sourceView != NULL) {
			// we have a source view; basic info about source
			sourceViewWidth = tsourcepView->getSourceWidth();
			sourceViewHeight = tsourcepView->getSourceHeight();
			//mydebug("in plugin_render 2b with source %s size: %dx%d outw = %d,%d.", tsourcepView->getName(), sourceViewWidth, sourceViewHeight, outputWidthAutomatic, outputHeightAutomatic);

			if (sourceViewWidth && sourceViewHeight) {
				// it's a valid source valid source

				// bypass mode says do NOTHING just render source to output (note this may resize it)
				// you can force this to always be true to bypass most processing for debug testing
				if (opt_filterBypass) {
					// just render out the source untouched, and quickly
					jrBlendClearMode blendClearMode = jrBlendClearMerge;
					jrRenderSourceOut(sourceView, sourceViewWidth, sourceViewHeight, getPluginOutputWidthAutomatic(), getPluginOutputHeightAutomatic(), blendClearMode);
				}
				else {
					// needs real rendering, just drop down
					needsRendering = true;
				}
			}
		}


		if (tsourcepTrack) {
			// tracking a different source
			sourceTrackWidth = tsourcepTrack->getSourceWidth();
			sourceTrackHeight = tsourcepTrack->getSourceHeight();
		}
		else {
			// we are tracking the same source as viewing
			sourceTrackWidth = sourceViewWidth;
			sourceTrackHeight = sourceViewHeight;
		}


		//mydebug("In dorender stage 3.");
		// this first test should always be true, unless source is NULL
		if (sourcePointerToTrack && (sourceTrackWidth && sourceTrackHeight)) {
			if (shouldUpdateTrackingBox) {
				trackedSourcePointerToTrack->findTrackingMarkerRegionInSource(sourcePointerToTrack, shouldUpdateTrackingBox, isHunting, false);
				didUpdateTrackingBox = true;
				if (oneShotEngaged) {
					oneShotDidAtLeastOneTrack = true;
				}
			}
		}


		// ATTN: WE *MUST* CALL ADVANCE HERE (early)-- otherwise if we try doing this at end of function, and in the meanwhile initiate a NEW hunt from tsourcepView->updateZoomCropBoxFromCurrentCandidate() we will advance over it before we test the first hunt index
		// this is awkward
		// advance hunt index -- but only after we track scan it (update rate may delay this for multiple cycles)
		if (didUpdateTrackingBox) {
			// if we were tracking something other than our own (ie we were hunting), then advance any hunting index
			if (tsourcepTrack) {
				//mydebug("Calling advance hunt...");
				stracker.AdvanceHuntToNextSource();
			}
		}


		bool needsUpdateZoomBox = true;
		if (needsUpdateZoomBox && sourceView) {
			// update target zoom box from tracking box -- note this may happen EVEN when we are not doing a tracking box update, to provide smooth movement to target
			// ATTN: BUT.. do we want to do this on tracking source or viewing source?
			// ATTN: new -- calling it on VIEW
			// ATTN: WARNING - this can trigger a new hunt, so make sure AdvanceHuntToNextSource is called first so we dont advance over new hunt's first index before the next cycle
			tsourcepView->updateZoomCropBoxFromCurrentCandidate(false);
		}

		//mydebug("In dorender stage 4.");

		// now final rendering (either with debug info, or with zoom crop effect, unless we bypasses above)
		if (needsRendering) {
			//mydebug("In dorender stage 4b.");
			// if we are in debugDisplayMode then we just display the debug overlay as our plugin and do not crop
			if (opt_debugRegions || opt_debugChroma) {
				// show EVERY source in a multiview format, with overlay debug info on internal chroma buffer; this uses TRACKED size
				//  then render from internal planning data texture to display for debug purposes
				// note that we do NOT force recalculation of boxes and marker detection here -- that is done only as normal, so the non-viewed sources here will be slow to update and seem freeze frames on their last tracked/hunted state
				// which is what we want so we can see how frequently they are being hunted.
				//mydebug("in plugin_render 4b did DEBUG on %s size: %dx%d.", trackedSourcePointerToTrack->getName(), sourceTrackWidth, sourceTrackHeight);
				if (true) {
					int sourceCount = stracker.getSourceCount();
					int dbgw = outputWidthPlugin / sourceCount;
					int dbgh = outputHeightPlugin;
					for (int i = 0; i < sourceCount; ++i) {
						TrackedSource* tsp = stracker.getTrackedSourceByIndex(i);

						if (opt_debugAllUpdate && shouldUpdateTrackingBox && tsp != trackedSourcePointerToTrack) {
							// force update of all sources in this debug mode
							obs_source_t* sp = tsp->borrowFullSourceFromWeakSource();
							if (sp) {
								tsp->findTrackingMarkerRegionInSource(sp, true, false, true);
								//tsp->updateZoomCropBoxFromCurrentCandidate(true);
								tsp->releaseBorrowedFullSource(sp);
							}
						}

						if (tsp && tsp->sourceWidth > 0) {
							// ok render this source into multiview (side by side)
							int dbgx1 = i * dbgw;
							int dbgy1 = 0;
							if (opt_debugRegions) {
								tsp->overlayDebugInfoOnInternalDataBuffer();
							}
							tsp->doRenderSourceWithInternalMemoryToFilterOutput(dbgx1, dbgy1, dbgw, dbgh, !opt_debugChroma);
						}
					}
				}
			}
			else if (DefBypassZoomOnUntouchedOutput && (!tsourcepView->lookingBoxReady || (tsourcepView->lookingx1 == 0 && tsourcepView->lookingy1 == 0 && tsourcepView->lookingx2 == outputWidthPlugin - 1 && tsourcepView->lookingy2 == outputHeightPlugin - 1))
				&& (tsourcepView->sourceWidth == outputWidthPlugin && tsourcepView->sourceHeight == outputHeightPlugin && outputWidthPlugin == outputWidthAutomatic && outputHeightPlugin == outputHeightAutomatic)) {
				//mydebug("in plugin_render 4c did render zoomcrop on %s size: %dx%d.", tsourcepView->getName(), sourceViewWidth, sourceViewHeight);
				// we could disable this check if it's not worth it -- we should make sure if occasionally gets used
				// this check is done here just to make things go faster in case of passing through
				// nothing to do at all -- either we have not decided a place to look OR we are looking at the entire view of a non-adjusted view, so nothing needs to be done to the view
				jrBlendClearMode blendClearMode = jrBlendClearMerge;
				jrRenderSourceOut(sourceView, sourceViewWidth, sourceViewHeight, outputWidthPlugin, outputHeightPlugin, blendClearMode);
			}
			else {
				// render the zoom and crop if any; THIS IS THE MAIN REAL ZOOMCROP RENDER
				if (fadeEndingSourceIndex == tsourcepView->index && updateFadePosition()) {
					// fade between some other source and tsourcepView
					tsourcepView->doRenderAutocropBoxFadedToScreen(sourceView, outputWidthPlugin, outputHeightPlugin, stracker.getTrackedSourceByIndex(fadeStartingSourceIndex), fadePosition);
				}
				else {
					// normal view
					//mydebug("Rendering normal view");
					tsourcepView->doRenderAutocropBoxToScreen(sourceView,outputWidthPlugin, outputHeightPlugin);
				}
			}
			needsRendering = false;
		}

		//mydebug("In dorender stage 5.");

		// this is a kludge needed for some obs bug(?) which does not update sources when we do a one-shot peek at them, so we have to continuously poll them on every cycle
		// new 9/7/22 im only doing this when we are on an updatecycle, so update rate will slow this down too
		if (DefKludgeTouchAllSourcesOnRenderCycle && shouldUpdateTrackingBox) {
			if (++kludgeTouchCounter >= DefKludgeTouchAllSourcesOnRenderCycleRate) {
				kludgeTouchCounter = 0;
				// this is a test to see if "touching" all other sources every cycle keeps them from getting stuck in old frames
				int sourceCount = stracker.getSourceCount();
				for (int i = 0; i < sourceCount; ++i) {
					TrackedSource* tsp = stracker.getTrackedSourceByIndex(i);
					if (true && tsp != tsourcepView && tsp != tsourcepTrack) {
						tsp->touchRefreshDuringRenderCycle();
					}
				}
			}
		}
	}

	// release source (it's ok if we pass NULL)
	if (tsourcepView) {
		tsourcepView->releaseBorrowedFullSource(sourceView); sourceView = NULL;
	}
	if (tsourcepTrack) {
		tsourcepTrack->releaseBorrowedFullSource(sourceTrack); sourceTrack = NULL;
	}

	if (isPluginTypeFilter()) {
		// clear the no-longer-trustworthy filter pointer to our source
		stracker.setExternallyManagedTrackedSource(0, NULL);
	}
}
//---------------------------------------------------------------------------






























//---------------------------------------------------------------------------
void JrPlugin::setEffectParamsChromaKey(uint32_t swidth, uint32_t sheight) {
	// setting params for effects file .effect
	struct vec2 pixel_size;
	vec2_set(&pixel_size, 1.0f / (float)swidth, 1.0f / (float)sheight);
	gs_effect_set_vec2(chroma_pixel_size_param, &pixel_size);
	//
	gs_effect_set_vec2(chroma1_param, &opt_chroma1);
	gs_effect_set_float(similarity1_param, opt_similarity1);
	gs_effect_set_float(smoothness1_param, opt_smoothness1);
	//
	gs_effect_set_vec2(chroma2_param, &opt_chroma2);
	gs_effect_set_float(similarity2_param, opt_similarity2);
	gs_effect_set_float(smoothness2_param, opt_smoothness2);
	gs_effect_set_float(testThreshold_param, opt_testThreshold);
}


void JrPlugin::setEffectParamsHsvKey(uint32_t swidth, uint32_t sheight) {
	// setting params for effects file .effect
	struct vec2 pixel_size;
	vec2_set(&pixel_size, 1.0f / (float)swidth, 1.0f / (float)sheight);
	gs_effect_set_vec2(hsv_pixel_size_param, &pixel_size);
	//
	gs_effect_set_float(hueThreshold1_param, opt_hueThreshold1);
	gs_effect_set_float(saturationThreshold1_param, opt_saturationThreshold1);
	gs_effect_set_float(valueThreshold1_param, opt_valueThreshold1);
	gs_effect_set_float(hueThreshold2_param, opt_hueThreshold2);
	gs_effect_set_float(saturationThreshold2_param, opt_saturationThreshold2);
	gs_effect_set_float(valueThreshold2_param, opt_valueThreshold2);
	//
	gs_effect_set_vec3(color1hsv_param, &color1AsHsv);
	gs_effect_set_vec3(color2hsv_param, &color2AsHsv);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrPlugin::setEffectParamsZoomCrop(uint32_t swidth, uint32_t sheight) {
	gs_effect_set_vec2(param_zoom_mul, &ep_zoom_mulVal);
	gs_effect_set_vec2(param_zoom_add, &ep_zoom_addVal);
	gs_effect_set_vec2(param_zoom_clip_ul, &ep_zoom_clip_ul);
	gs_effect_set_vec2(param_zoom_clip_lr, &ep_zoom_clip_lr);
	gs_effect_set_vec2(param_zoom_hardClip_ul, &ep_zoom_hardClip_ul);
	gs_effect_set_vec2(param_zoom_hardClip_lr, &ep_zoom_hardClip_lr);
	//
	struct vec2 pixel_size;
	vec2_set(&pixel_size, 1.0f / (float)swidth, 1.0f / (float)sheight);
	gs_effect_set_vec2(param_zoom_pixel_size, &pixel_size);
}


void JrPlugin::setEffectParamsFade(uint32_t swidth, uint32_t sheight) {
	gs_texture_t *texaInput = gs_texrender_get_texture(fadeTexRenderA);
	gs_texture_t *texbInput = gs_texrender_get_texture(fadeTexRenderB);
	if (!texaInput) {
		mydebug("ERROR texaInput null");
	}
	if (!texbInput) {
		mydebug("ERROR texbInput null");
	}

	gs_effect_set_texture(param_fade_a, texaInput);
	gs_effect_set_texture(param_fade_b, texbInput);
	gs_effect_set_float(param_fade_val, (float)(1.0-fadePosition));
}



void JrPlugin::setEffectParamsOutput(TrackedSource* tsourcep, uint32_t swidth, uint32_t sheight, int passNumber, gs_texrender_t* secondaryTexture) {
	// calculated in zoombox calculations
	//
	gs_effect_set_vec2(param_output_clip_ul, &ep_output_clip_ul);
	gs_effect_set_vec2(param_output_clip_lr, &ep_output_clip_lr);
	gs_effect_set_vec2(param_output_hardClip_ul, &ep_output_hardClip_ul);
	gs_effect_set_vec2(param_output_hardClip_lr, &ep_output_hardClip_lr);
	//
	gs_effect_set_bool(param_output_effectInside, false);
	gs_effect_set_int(param_output_passNumber, passNumber);
	//
	if (secondaryTexture != NULL) {
		gs_texture_t* texInput = gs_texrender_get_texture(secondaryTexture);
		if (!texInput) {
			mydebug("ERROR texaInput null");
		}
		gs_effect_set_texture(param_ouput_secondaryTex, texInput);
	}
	else {
		gs_effect_set_texture(param_ouput_secondaryTex, NULL);
	}

	gs_effect_set_float(param_ouput_exteriorDullness, ep_optBlurExteriorDullness);
	//
	struct vec2 pixel_size;
	vec2_set(&pixel_size, 1.0f / (float)swidth, 1.0f / (float)sheight);
	gs_effect_set_vec2(param_output_pixel_size, &pixel_size);
}



void JrPlugin::setEffectParamsDilate(uint32_t swidth, uint32_t sheight) {
	// setting params for effects file .effect
	struct vec2 pixel_size;
	vec2_set(&pixel_size, 1.0f / (float)swidth, 1.0f / (float)sheight);
	gs_effect_set_vec2(param_dilate_pixel_size, &pixel_size);
	//
	gs_effect_set_vec4(param_dilateBackgroundRgba, &colorBackgroundAsRgbaVec);
}
//---------------------------------------------------------------------------





























//---------------------------------------------------------------------------
bool JrPlugin::doCalculationsForZoomCropEffect(TrackedSource* tsourcep) {
	// this code need refactoring
	// a good part of this code is dealing with situation when we have a forced output size different from the full size of our source

	float sourceWidth = (float)tsourcep->getSourceWidth();
	float sourceHeight = (float)tsourcep->getSourceHeight();

	float oWidth = (float)outputWidthPlugin;
	float oHeight = (float)outputHeightPlugin;

	// helpers
	float scaleModX = oWidth / sourceWidth;
	float scaleModY = oHeight / sourceHeight;

	// sanity checks
	if (sourceWidth <= 0 || sourceHeight <= 0 || oWidth<=0 || oHeight<=0) {
		return false;
	}

	// target region size
	int bx1 = tsourcep->lookingx1;
	int by1 = tsourcep->lookingy1;
	int bx2 = tsourcep->lookingx2;
	int by2 = tsourcep->lookingy2;

	//info("Debugging b vals: %d,%d,%d,%d.", bx1, by1, bx2, by2);

	int cropWidth = (bx2-bx1);
	int cropHeight = (by2-by1);

	// sanity check
	if (cropWidth <= 1 || cropHeight <= 1) {
		return false;
	}

	// hard clipping covers entire image source
	vec2_zero(&ep_zoom_hardClip_ul);
	ep_zoom_hardClip_lr.x = 1.0f;
	ep_zoom_hardClip_lr.y = 1.0f;

	// actual effect clipping crop is simple, either none or around box
	// clipping in 0-1 scale (default to none)
	if (opt_zcMode == Def_zcMode_OnlyZoom) {
		// no crop clipping, so we just set boundary of entire source; i dont think this does any cropping, but note we modify this below near the end when we check for this mode again
		vec2_zero(&ep_zoom_clip_ul);
		ep_zoom_clip_lr.x = 1.0f;
		ep_zoom_clip_lr.y = 1.0f;
	} else {
		// both other modes clip to looking box (bx1 etc)
		// crop clip box
		ep_zoom_clip_ul.x = (float)bx1 / (float)sourceWidth;
		ep_zoom_clip_lr.x = (float)(bx2) / (float)sourceWidth;
		ep_zoom_clip_ul.y = (float)by1 / (float)sourceHeight;
		ep_zoom_clip_lr.y = (float)(by2) / (float)sourceHeight;
	}


	// allignment prep
	int halign[9] = { -1,0,1,-1,0,1,-1,0,1 };
	int valign[9] = { -1,-1,-1,0,0,0,1,1,1 };
	int opt_halign = halign[opt_zcAlign];
	int opt_valign = valign[opt_zcAlign];


	// ok now we calculate scaling
	// if full zoom and no aspect ratio
	float scaleX = (float)cropWidth / oWidth;
	float scaleY = (float)cropHeight / oHeight;
	float oscaleX, oscaleY;

	// enforce max zoom limit?
	if (opt_zcMode == Def_zcMode_OnlyCrop) {
		// no zoom (just crop)
		//scaleX = scaleY = 1.0f;
		cropWidth = (int)sourceWidth;
		cropHeight = (int)sourceHeight;
		scaleX = (float)sourceWidth / oWidth;
		scaleY = (float)sourceHeight / oHeight;
	}

	//mydebug("In doCalculationsForZoomCropEffect output size: %f,%f  sourcewidth=%f  sourceheight=%f   cropw=%d  croph=%d looking= %d,%d,%d,%d.", oWidth, oHeight, sourceWidth, sourceHeight,cropWidth,cropHeight, bx1,by1,bx2,by2);

	//
	if (opt_zcMaxZoom > 0.01f) {
		// max zoom cap
		float zoomScale = (float)min(1.0 / opt_zcMaxZoom,1.0f);
		scaleX = max(scaleX, zoomScale);
		scaleY = max(scaleY, zoomScale);
	}

	// now adjust for aspect ratio constraints -- force them to be same zoom, whichever is LESS (so more will be shown in one dimension than we wanted)
	if (opt_zcPreserveAspectRatio) {
		if (scaleX > scaleY) {
			scaleY = scaleX;
		}
		else if (scaleY > scaleX) {
			scaleX = scaleY;
		}
	}

	// now adjust final output scale for our outputscaling rendering
	oscaleX = scaleX * scaleModX;
	oscaleY = scaleY * scaleModY;

	// effect scaling -- this is just scale, modified for our output rescaling size
	ep_zoom_mulVal.x = oscaleX;
	ep_zoom_mulVal.y = oscaleY;

	// alignment is the more tricky part..
	// calculate add offset value

	float leftPixelX, topPixelY;

	// ok now that we have zoom scale we can calculate extents of our area of interest
	float resizedWidth = (float)cropWidth / scaleX;
	float resizedHeight = (float)cropHeight / scaleY;
	// and the amount of space around the zoomed area (should be 0 on full non-aspect constrained zoom)
	float surroundX = oWidth - resizedWidth;
	float surroundY = oHeight - resizedHeight;

	// align

	// calculation of top left offset for drawing
	if (opt_halign == -1) {
		// left
		leftPixelX = 0;
	}
	else if (opt_halign == 0) {
		// center
		leftPixelX = surroundX / 2.0f;
	}
	else if (opt_halign == 1) {
		// right
		leftPixelX = surroundX;
	}
	if (opt_valign == -1) {
		// top
		topPixelY = 0;
	}
	else if (opt_valign == 0) {
		// center
		topPixelY = surroundY / 2.0f;
	}
	else if (opt_valign == 1) {
		// bottom
		topPixelY = surroundY;
	}
	// ok now set offsets
	vec2_zero(&ep_zoom_addVal);
	if (opt_zcMode == Def_zcMode_OnlyCrop) {
		// only crop
		ep_zoom_addVal.x = (( - leftPixelX * scaleX) / (float)sourceWidth);
		ep_zoom_addVal.y = (( - topPixelY * scaleY) / (float)sourceHeight);
	}
	else {
		// zoomcrop or only zoom
		ep_zoom_addVal.x = ((bx1 - leftPixelX * scaleX) / (float)sourceWidth);
		ep_zoom_addVal.y = ((by1 - topPixelY * scaleY) / (float)sourceHeight);
	}
	//


	// this is for our new blur mode but also for pure zoom mode, where we show stuff OUTSIDE the actual looking marker box
	if (true) {
		float cropXextra = (surroundX - leftPixelX) * scaleX;
		float cropYextra = (surroundY - topPixelY) * scaleY;
		if (opt_zcMode == Def_zcMode_OnlyZoom) {
			// only zoom
			// in this mode there will be NO cropping to the output screen area, it will be filled explicitly
			// ATTN: unless zoom size is maxed out
			// cropping clipping in zoom mode normally crops to source size but if window output is forced smalller we need to clip to that
			// ATTN: i have forgotten what this is useful for
			vec2_zero(&ep_zoom_hardClip_ul);
			ep_zoom_clip_lr.x = (float)min(1.0, ((float)bx2 + cropXextra) / sourceWidth);
			ep_zoom_clip_lr.y = (float)min(1.0, ((float)by2 + cropYextra) / sourceHeight);
		} else if (opt_zcMode == Def_zcMode_OnlyCrop) {
			// crop mode shows full 
			vec2_zero(&ep_zoom_hardClip_ul);
			ep_zoom_hardClip_lr.x = 1.0;
		} else {
			// there may be cropping (if aspect ratio is preserved, or if zoom is limited)
			// if in blur mode, we will use these values as calculated above for zoom-only mode as our hard clips
			vec2_zero(&ep_zoom_hardClip_ul);
			ep_zoom_hardClip_lr.x = (float)min(1.0, ((float)bx2 + cropXextra) / sourceWidth);
			ep_zoom_hardClip_lr.y = (float)min(1.0, ((float)by2 + cropYextra) / sourceHeight);
		}
	}

	// calculate resulting output soft clip locations for post effect
	// these should be in 0-1 coordinates for OUTPUT resulting transformation
	// this is inverting the operation in the filter
	// PAINFUL
	ep_output_clip_ul.x = (ep_zoom_clip_ul.x - ep_zoom_addVal.x) / ep_zoom_mulVal.x;
	ep_output_clip_ul.y = (ep_zoom_clip_ul.y - ep_zoom_addVal.y) / ep_zoom_mulVal.y;
	ep_output_clip_lr.x = (ep_zoom_clip_lr.x - ep_zoom_addVal.x) / ep_zoom_mulVal.x;
	ep_output_clip_lr.y = (ep_zoom_clip_lr.y - ep_zoom_addVal.y) / ep_zoom_mulVal.y;
	// clip entire source
	ep_output_hardClip_ul.x = (0 - ep_zoom_addVal.x) / ep_zoom_mulVal.x;
	ep_output_hardClip_ul.y = (0 - ep_zoom_addVal.y) / ep_zoom_mulVal.y;
	ep_output_hardClip_lr.x = (1.0f - ep_zoom_addVal.x) / ep_zoom_mulVal.x;
	ep_output_hardClip_lr.y = (1.0f - ep_zoom_addVal.y) / ep_zoom_mulVal.y;

	return true;
}
//---------------------------------------------------------------------------









