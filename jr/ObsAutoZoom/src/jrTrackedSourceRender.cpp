//---------------------------------------------------------------------------
#include "jrSourceTracker.h"
#include "jrPlugin.h"
#include "obsHelpers.h"
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void TrackedSource::doRenderWorkFromEffectToStageTexRender(gs_effect_t* effectChroma, obs_source_t* source) {
	JrPlugin* plugin = getPluginp();

	//mydebug("In doRenderWorkFromEffectToStageTexRender1.");

	if (stageHoldingSpaceTexrender == NULL) {
		mydebug("!!!!!!!!!!!!!!!!!!!!!! WARNING, stageHoldingSpaceTexrender is null!");
	}
	if (stagingTexrender == NULL) {
		mydebug("!!!!!!!!!!!!!!!!!!!!!! WARNING, stagingTexrender is null!");
	}

	// STAGE 1 render SOURCE
	// render into holdingSpaceTexrender (temporary) texture - stretched to a reduced stage x stage dimension
	jrBlendClearMode blendClearMode = jrBlendClearOverwite;
	
	
	bool flagRenderForChromaAtReducedStageSize = true;
	if (flagRenderForChromaAtReducedStageSize) {
		jrRenderSourceIntoTextureAtSizeLoc(source, stageHoldingSpaceTexrender, sourceWidth, sourceHeight, 0, 0, stageWidth, stageHeight, blendClearMode, false);
	}
	else {
		jrRenderSourceIntoTextureAtSizeLoc(source, stageHoldingSpaceTexrender, sourceWidth, sourceHeight, 0, 0, sourceWidth, sourceHeight, blendClearMode, false);
	}

	//mydebug("In doRenderWorkFromEffectToStageTexRender 2.");
	//mydebug("In doRenderWorkFromEffectToStageTexRender 2 (%d,%d) format %d.", stageHoldingSpaceTexrender->width, stageHoldingSpaceTexrender->height, stageHoldingSpaceTexrender->format);

	// STAGE 2 - render effect from this holdingSpaceTexrender to stagingTexRender, with chroma effect applied
	// first set effect params
	plugin->setEffectParamsChroma(sourceWidth, sourceHeight);
	// then render effect from holding space into stage
	blendClearMode = jrBlendClearOverwite;
	char* drawTechnique = markerChromaModeRenderTechniques[plugin->getOptMarkerChromaMode()];
	jrRenderEffectIntoTexture(stagingTexrender, effectChroma, stageHoldingSpaceTexrender, stageWidth, stageHeight, blendClearMode, drawTechnique);

	//mydebug("In doRenderWorkFromEffectToStageTexRender 3.");
	//mydebug("In doRenderWorkFromEffectToStageTexRender 3 (%d,%d) format %d.", stagingTexrender->width, stagingTexrender->height, stagingTexrender->format);;
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// we are getting an obs crash here regarding state surface
// the function called before us that might be corrupting the stage is doRenderWorkFromEffectToStageTexRender(
bool TrackedSource::doRenderWorkFromStageToInternalMemory() {
	JrPlugin* plugin = getPluginp();
	bool stageMemoryReady = false;

	//mydebug("doRenderWorkFromStageToInternalMemory START");

	// clear it to help debugger not see old version
	if (DefDebugClearDrawingSpaced) {
		memset(stagedData, 0x07, (stageWidth + 32) * stageHeight * 4);
	}

	if (stagedData) {
		//mydebug("doRenderWorkFromStageToInternalMemory 2");
		// default OBS plugin since we don't want to do anything
		gs_effect_t *effectDefault = obs_get_base_effect(OBS_EFFECT_DEFAULT);
		gs_texture_t *tex = gs_texrender_get_texture(stagingTexrender);
		if (tex) {
			//mydebug("doRenderWorkFromStageToInternalMemory 3");

			gs_stage_texture(stagingSurface, tex);
			//
			uint8_t *idata;
			uint32_t ilinesize;
			WaitForSingleObject(plugin->mutex, INFINITE);
			//mydebug("doRenderWorkFromStageToInternalMemory 4");
			if (gs_stagesurface_map(stagingSurface, &idata, &ilinesize)) {
				//mydebug("ATTN: JR gs_stage_texture 2.\n");
				//mydebug("doRenderWorkFromStageToInternalMemory 5");
				memcpy(stagedData, idata, ilinesize * stageHeight);
				// update our stored data linesize
				stagedDataLineSize = ilinesize;
				//mydebug("doRenderWorkFromStageToInternalMemory 6");
				gs_stagesurface_unmap(stagingSurface);
				stageMemoryReady = true;
				//mydebug("doRenderWorkFromStageToInternalMemory 7");
			} else {
				mydebug("ERROR ------> doRenderWorkFromStageToInternalMemory failed in call to gs_stagesurface_map.");
			}
			//mydebug("doRenderWorkFromStageToInternalMemory 8");
			ReleaseMutex(plugin->mutex);
			//mydebug("doRenderWorkFromStageToInternalMemory 9");
		}
		else {
			mydebug("ERROR ---> doRenderWorkFromStageToInternalMemory tex is NULL.");
		}
	} else {
		mydebug("ERROR ---> doRenderWorkFromStageToInternalMemory stagedata is NULL.");
	}

	//mydebug("doRenderWorkFromStageToInternalMemory 10");
	if (!stageMemoryReady) {
		mydebug("ERROR ---> stageMemoryReady is false.");
		// clear it to help debugger not see old version
		if (DefDebugClearDrawingSpaced) {
			memset(stagedData, 0xFF, (stageWidth + 32) * stageHeight * 4);
		}
	}

	//mydebug("doRenderWorkFromStageToInternalMemory DONE");
	return true;
}
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
void TrackedSource::overlayDebugInfoOnInternalDataBuffer() {
	// overlay debug 
	JrPlugin* plugin = getPluginp();
	int x1, y1, x2, y2;
	int pixelVal;

	//mydebug("overlayDebugInfoOnInternalDataBuffer for index %d.", index);

	if (true) {
		// show all regions
		JrRegionSummary* region;
		JrRegionDetector* rdp = &regionDetector;
		for (int i = 0; i < rdp->foundRegions; ++i) {
			region = rdp->getRegionpByIndex(i);
			pixelVal = calcIsValidmarkerRegion(region) ?   0xFFFF0000 : 0xFF000099;
			x1 = region->x1 - 1;
			y1 = region->y1 - 1;
			x2 = region->x2 + 1;
			y2 = region->y2 + 1;
			//
			if (x1 < 0) { x1 = 0; }
			if (y1 < 0) { y1 = 0; }
			if (x2 >= stageWidth) { x2 = stageWidth - 1; }
			if (y2 >= stageHeight) { y2 = stageHeight - 1; }
			//
			jrRgbaDrawRectangle((uint32_t*)stagedData, stagedDataLineSize, x1, y1, x2, y2, pixelVal);
		}
	}

	// show found cropable area
	if (areMarkersBothVisibleOrOneOccluded()) {
		// modify contents of our internal data to show border around box
		//mydebug("Overlaying tracking box (%d,%d,%d,%d).", markerx1, markery1, markerx2, markery2);
		// this is the yellow box
		pixelVal = (index == sourceTrackerp->getViewSourceIndex()) ? 0xFFFFFFFF : 0xFFFF00FF;

		// just tweak offset a bit
		x1 = (int)(markerx1-stageShrinkX);
		y1 = (int)(markery1-stageShrinkY);
		x2 = (int)(markerx2+stageShrinkX);
		y2 = (int)(markery2+stageShrinkY);
		//
		if (x1 < 0) { x1 = 0; }
		if (y1 < 0) { y1 = 0; }
		if ((unsigned int)x2 >= sourceWidth) { x2 = sourceWidth - 1; }
		if ((unsigned int)y2 >= sourceHeight) { y2 = sourceHeight - 1; }	
		jrRgbaDrawRectangle((uint32_t*)stagedData, stagedDataLineSize, cnvSourceToStageX(x1), cnvSourceToStageY(y1), cnvSourceToStageX(x2), cnvSourceToStageY(y2), pixelVal);

		if (!areMarkersBothVisibleNeitherOccluded()) {
			// draw somethign to let us know one marker is excluded
			int ix1 = x1;
			int iy1 = y1;
			int ix2 = x2;
			int iy2 = y2;
			// just tweak offsets a bit so rectangle is visible
			int pdimx = (int)(stageShrinkX * 4);
			int pdimy = (int)(stageShrinkY * 4);
			// shouldnt happen
			ix1 = (x1+x2)/2 - pdimx;
			ix2 = (x1+x2)/2 + pdimx;
			iy1 = (y1+y2)/2 - pdimy;
			iy2 = (y1+y2)/2 + pdimy;
			if (ix1 < 0) { ix1 = 0; }
			if (iy1 < 0) { iy1 = 0; }
			if ((unsigned int)ix2 >= sourceWidth) { ix2 = sourceWidth - 1; }
			if ((unsigned int)iy2 >= sourceHeight) { iy2 = sourceHeight - 1; }			
			
			jrRgbaDrawRectangle((uint32_t*)stagedData, stagedDataLineSize, cnvSourceToStageX(ix1), cnvSourceToStageY(iy1), cnvSourceToStageX(ix2), cnvSourceToStageY(iy2), pixelVal);
		}
	}

	unsigned long difThreshold = 10;
	if (lookingBoxReady && (fabs(markerx1-lookingx1) + fabs(markery1-lookingy1) + fabs(markerx2-lookingx2) + fabs(markery2-lookingy2) > difThreshold)) {
		// modify contents of our internal data to show border around box
		//mydebug("Overlaying crop box (%d,%d,%d,%d).", bx1, by1, bx2, by2);
		pixelVal = (index == sourceTrackerp->getViewSourceIndex()) ? 0xFFAAAA00 : 0xFF777700;

		x1 = lookingx1;
		y1 = lookingy1;
		x2 = lookingx2;
		y2 = lookingy2;
		//
		if (x1 < 0) { x1 = 0; }
		if (y1 < 0) { y1 = 0; }
		if ((unsigned int)x2 > sourceWidth-1) {
			x2 = sourceWidth - 1;
		}
		if ((unsigned int)y2 > sourceHeight-1) {
			y2 = sourceHeight - 1;
		}
		jrRgbaDrawRectangle((uint32_t*)stagedData, stagedDataLineSize, cnvSourceToStageX(x1), cnvSourceToStageY(y1), cnvSourceToStageX(x2), cnvSourceToStageY(y2), pixelVal);
	}

	if (index == sourceTrackerp->getViewSourceIndex()) {
		// show active source with big box around entire view
		pixelVal = 0xFFFFFFFF;
		int margin = 2;
		x1 = 0+margin;
		y1 = 0+margin;
		x2 = sourceWidth-margin;
		y2 = sourceHeight-margin;
		//
		if (x1 < 0) { x1 = 0; }
		if (y1 < 0) { y1 = 0; }
		if ((unsigned int)x2 > sourceWidth-1) {
			x2 = sourceWidth - 1;
		}
		if ((unsigned int)y2 > sourceHeight-1) {
			y2 = sourceHeight - 1;
		}
		jrRgbaDrawRectangle((uint32_t*)stagedData, stagedDataLineSize, cnvSourceToStageX(x1), cnvSourceToStageY(y1), cnvSourceToStageX(x2), cnvSourceToStageY(y2), pixelVal);

	}

}
//---------------------------------------------------------------------------
	
	















































































//---------------------------------------------------------------------------
void TrackedSource::doRenderAutocropBoxToScreen( obs_source_t* source, int owidth, int oheight) {
	//mydebug("In doRenderAutocropBoxToScreen testing with source size = %d,%d and out is %d,%d.", sourceWidth, sourceHeight, owidth, oheight);

	// STAGE 1 render SOURCE into holding space
	// render into texrender2 (temporary) texture - of SAME size as source
	// ATTN: why do we need to to this? can't we just run the effect directly on the source?
	jrBlendClearMode blendClearMode = jrBlendClearOverwite;
	jrRenderSourceIntoTexture(source, sourceHoldingSpaceTexrender, sourceWidth, sourceHeight, blendClearMode);

	// STAGE 2 - render effect on this sourceHoldingSpaceTexrender sending output to screen output
	doRenderAutocropBoxIntoTextureFromHoldingSpace(NULL, owidth, oheight);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void TrackedSource::doRenderAutocropBoxFadedToScreen( obs_source_t* source, int owidth, int oheight, TrackedSource* fromtsp, float fadePosition) {
	// crop zoom effect with fade
	JrPlugin* plugin = getPluginp();

	//mydebug("In doRenderAutocropBoxFadedToScreen with index %d and fadevel = %f", index, fadeVal);
	float fadeVal = plugin->fadePosition;
	if (fadeVal < 0.1f) {
		// close enough to done that we dont need to bother
		doRenderAutocropBoxToScreen(source, owidth, oheight);
		return;
	}

	// render source A into texture A
	// note that we use the ALREADY rendered into fromtsp->sourceHoldingSpaceTexrender which is like a screenshot and should be valid if we are fading from it
	// so fromtsp->sourceHoldingSpaceTexrender is considered valid
	fromtsp->doRenderAutocropBoxIntoTextureFromHoldingSpace(plugin->fadeTexRenderA, owidth, oheight);

	// ok so now we should have a texture with the zoomed in image from the fromtsp (you could even save cpu by screenshotting THIS rendered autozoomed image and not updating each each moment of fade
	// and now we would like to plend this in with output

	// now render source B (us) into texture B (this matches code in doRenderAutocropBoxToScreen)
	// 1. first render latest version of source into our sourceHoldingSpaceTexrender
	jrRenderSourceIntoTexture(source, sourceHoldingSpaceTexrender, sourceWidth, sourceHeight, jrBlendClearOverwite);
	// 2. now render us into texture B
	doRenderAutocropBoxIntoTextureFromHoldingSpace(plugin->fadeTexRenderB, owidth, oheight);

	// set effects params for the fade
	plugin->setEffectParamsFade(sourceWidth, sourceHeight);

	// ok so now we have texture A and texture B the two images to fade between
	// and now we would like to blend these two textures into a final output shown on screeen
	// note that this goes from iwidth size to final outwidth
	// ATTN: why do we use BlendClearMerge instead of blendClearOverwrite??
	//jrBlendClearMode blendClearMode = jrBlendClearMerge;
	jrBlendClearMode blendClearMode = jrBlendClearOverwite;
	jrRenderConfiguredEffectIntoTextureAtSize(NULL, plugin->effectFade, owidth, oheight, owidth, oheight, blendClearMode, "FadeLinear");
}
//---------------------------------------------------------------------------










































//---------------------------------------------------------------------------
void TrackedSource::doRenderFromInternalMemoryToFilterOutput(int outx1, int outy1, int outWidth, int outHeight) {
	// copy our data modified pixels into a texture to render
	//mydebug("doRenderFromInternalMemoryToFilterOutput index %d (%d,%d) to target %d,%d-%d,%d.", index, stageWidth, stageHeight,outx1,outy1,outx2,outy2);

	// test write out to drawing texture for plugin output
	// see https://gitlab.hs-bremerhaven.de/jpeters/obs-studio/-/blob/master/test/test-input/sync-pair-vid.c
	// see https://discourse.urho3d.io/t/urho3d-as-plugin-for-obs-studio/6849/7
	uint8_t *ptr;
	uint32_t ilinesize;
	// copy from our internal buffer to the output texture in RGBA format
	if (gs_texture_map(stageDrawingTexture, &ptr, &ilinesize)) {
		memcpy((uint32_t*)ptr, stagedData, ilinesize * stageHeight);
		gs_texture_unmap(stageDrawingTexture);
	}

	// ok render texture from internal memory onto screen at FORCED size (fine to squish it)
	// false as last parameter says dont clear the output when rendering so we can overlay multiple debug overlays on same screen
	gs_effect_t *effectDefault = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	jrBlendClearMode blendClearMode = jrBlendOverwriteMerge;
	jrRenderEffectIntoTextureAtSizeLoc(NULL, effectDefault, NULL, stageDrawingTexture, stageWidth, stageHeight, outx1, outy1, outWidth, outHeight, blendClearMode, "Draw");
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void TrackedSource::doRenderSourceWithInternalMemoryToFilterOutput(int outx1, int outy1, int outWidth, int outHeight, bool optionShowOnTopOfSource) {
	// this is a replacement for doRenderFromInternalMemoryToFilterOutput which merges debug overlayed onto source instead of JUST displaying internal
	// //
	// copy our data modified pixels into a texture to render
	//mydebug("doRenderFromInternalMemoryToFilterOutput index %d (%d,%d) to target %d,%d-%d,%d.", index, stageWidth, stageHeight,outx1,outy1,outx2,outy2);
	jrBlendClearMode blendModeDebug = jrBlendOverwriteMerge;

	// first we render SOURCE
	if (optionShowOnTopOfSource) {
		// lease it
		obs_source_t* osp = borrowFullSourceFromWeakSource();
		// resize check
		if (osp) {
			recheckSizeAndAdjustIfNeeded(osp);
		}
		if (sourceWidth > 0) {
			// draw it
			jrBlendClearMode blendClearMode = jrBlendOverwriteMerge;
			jrRenderSourceIntoTextureAtSizeLoc(osp, NULL, sourceWidth, sourceHeight, outx1, outy1, outWidth, outHeight, blendClearMode, true);
			// render debug on top
			blendModeDebug = jrBlendDebugOverlay;
		}
		// release full source
		releaseBorrowedFullSource(osp);
	}

	// now our internal annotated memory on top
	uint8_t *ptr;
	uint32_t ilinesize;
	// copy from our internal buffer to the output texture in RGBA format
	if (gs_texture_map(stageDrawingTexture, &ptr, &ilinesize)) {
		memcpy((uint32_t*)ptr, stagedData, ilinesize * stageHeight);
		gs_texture_unmap(stageDrawingTexture);
	}

	// ok render texture from internal memory onto screen at FORCED size (fine to squish it)
	// false as last parameter says dont clear the output when rendering so we can overlay multiple debug overlays on same screen
	gs_effect_t *effectDefault = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	jrRenderEffectIntoTextureAtSizeLoc(NULL, effectDefault, NULL, stageDrawingTexture, stageWidth, stageHeight, outx1, outy1, outWidth, outHeight, blendModeDebug, "Draw");
}
//---------------------------------------------------------------------------




















//---------------------------------------------------------------------------
void TrackedSource::doRenderAutocropBoxIntoTextureFromHoldingSpace(gs_texrender_t* texRender, int owidth, int oheight) {
	// helper render autocrop box
	// remember that texRender will be NULL when rendering to screen
	// ATTN: this is the centralized function where we render the zoom view, so this where we would insert additional steps into pipeline if we wanted to perform some effects on the output before showing
	// e.g. normally we render from "holdingSpace -> texRender", but to inject another effect in pipeline we would go "HoldingSpace -> tempOut -> texRender"

	gs_texrender_t* curTexRender = texRender;
	bool addPipelineEffects = true;

	// add any special effects?
	bool optionEffectAddBlur = false;
	if (plugin->opt_zcCropStyle == Def_zcCropStyle_blur && plugin->opt_zcMode!=Def_zcMode_OnlyZoom) {
		optionEffectAddBlur = true;
	}

	if (optionEffectAddBlur) {
		// lets test it out
		// set intermediate target
		curTexRender = plugin->outTexRenderBase;
	}

	// render zoom crop effect
	plugin->doCalculationsForZoomCropEffect(this);
	plugin->setEffectParamsZoomCrop(sourceWidth, sourceHeight);
	jrRenderEffectIntoTextureAtSizeLoc(curTexRender, plugin->effectZoomCrop, sourceHoldingSpaceTexrender, NULL, sourceWidth, sourceHeight, 0, 0, owidth, oheight, jrBlendClearOverwite, plugin->cropZoomTechnique);

	if (optionEffectAddBlur) {
		// render from curTexRender to next target using next pipeline in chain
		// this function should be called with either plugin->outTexRenderA or plugin->outTexRenderB, and it will go from one to the other and return the destination
		curTexRender = doPipeLineOutputEffect_Blur(curTexRender, owidth, oheight);
	}

	// here we could add more effects to the pipeline..

	// final texRender if different
	if (curTexRender != texRender) {
		jrRenderTextureIntoTexture(texRender, curTexRender, owidth, oheight, jrBlendClearOverwite);
	}
}
//---------------------------------------------------------------------------

















//---------------------------------------------------------------------------
gs_texrender_t* TrackedSource::doPipeLineOutputEffect_Blur(gs_texrender_t* inputTexRender, int owidth, int oheight) {
	gs_texrender_t* outputTextRenderLastOut = inputTexRender;
	gs_texrender_t* outputTextRenderNextOut = (outputTextRenderLastOut == plugin->outTexRenderA) ? plugin->outTexRenderB : plugin->outTexRenderA;
	bool flagNeedsMergedOriginal = false;
	bool flagChangedSize = false;

	if (false) {
		// test -- just render from input to output texture without running any effect
		jrRenderTextureIntoTexture(outputTextRenderNextOut, outputTextRenderLastOut, owidth, oheight, jrBlendClearOverwite);
		outputTextRenderLastOut = outputTextRenderNextOut;
		outputTextRenderNextOut = (outputTextRenderLastOut == plugin->outTexRenderA) ? plugin->outTexRenderB : plugin->outTexRenderA;
	} else {
		// blur
		int numpasses = plugin->ep_optBlurPasses;
		float downRatio = plugin->ep_optBlurSizeReduction;
		//
		if (downRatio>1.0f) {
			flagChangedSize = true;
		}
		//
		for (int i=0;i<numpasses;++i) {
			plugin->setEffectParamsOutput(this, owidth, oheight, i, NULL);
			// run effect
			jrRenderEffectIntoTexture(outputTextRenderNextOut, plugin->effectOutput, outputTextRenderLastOut, (int)(owidth/downRatio), (int)(oheight/downRatio), jrBlendClearOverwite, "Blur");
			// advance for next pass
			outputTextRenderLastOut = outputTextRenderNextOut;
			outputTextRenderNextOut = (outputTextRenderLastOut == plugin->outTexRenderA) ? plugin->outTexRenderB : plugin->outTexRenderA;
		}
		if (downRatio > 0) {
			flagNeedsMergedOriginal = true;
		}
	}

	if (flagNeedsMergedOriginal) {
		// we have done things that have corrupted our interior softcrop region, so we need to overwrite it
		if (true) {
			// render at full scale -- we could skip this if we thought it was already at full scale (check texture size)
			jrRenderTextureIntoTexture(outputTextRenderNextOut, outputTextRenderLastOut, owidth, oheight, jrBlendClearOverwite);
			// advance for next pass
			outputTextRenderLastOut = outputTextRenderNextOut;
			outputTextRenderNextOut = (outputTextRenderLastOut == plugin->outTexRenderA) ? plugin->outTexRenderB : plugin->outTexRenderA;
		}

		if (flagChangedSize) {
			// now override softcrop area with original
			plugin->setEffectParamsOutput(this, owidth, oheight, 0, outputTextRenderLastOut);
			// run effect on original input
			if (outputTextRenderNextOut == inputTexRender) {
				mydebug("ERROR trying to blur effect from and to same texture.");
				return outputTextRenderLastOut;
			}
			jrRenderEffectIntoTexture(outputTextRenderNextOut, plugin->effectOutput, inputTexRender, owidth, oheight, jrBlendClearOverwite, "DrawInteriorExterior");
			// advance for next pass
			outputTextRenderLastOut = outputTextRenderNextOut;
			outputTextRenderNextOut = (outputTextRenderLastOut == plugin->outTexRenderA) ? plugin->outTexRenderB : plugin->outTexRenderA;
		}
	}

	// and return output texture
	return outputTextRenderLastOut;
}
//---------------------------------------------------------------------------
