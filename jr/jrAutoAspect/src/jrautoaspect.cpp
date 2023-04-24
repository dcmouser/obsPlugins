/*
ObsScrenFlip - flips left and right halves of screen (configurable where) so that when you read from preview screen
it looks like you are looking off in the other direction.
*/

#include "pluginInfo.hpp"
//
#include "jrautoaspect.hpp"

// this MUST come after pluginInfo included, as it depends on macros there
#include "../../jrcommon/src/jrobsplugin_source_globalfuncs.hpp"

#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>

#include <string>
#include <vector>


//---------------------------------------------------------------------------
// forward declarations that are extern "C"
#ifdef __cplusplus
extern "C" {
#endif
	bool OnPropertyChangeCallback(obs_properties_t* props, obs_property_t* p, obs_data_t* settings);
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool OnPropertyChangeCallback(obs_properties_t* props, obs_property_t* p, obs_data_t* settings) {
	UNUSED_PARAMETER(p);
	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
const char* JrOrientationMode_Choices[] = { "portrait", "landscape", "auto", NULL};
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// global singleton source info
struct obs_source_info SingletonSourcePluginInfoJrAutoAspect_Filter;
//
void JrAutoAspect::gon_pluginModuleSingletonLoadDoRegistration() {
	gon_pluginModuleSingletonLoadDoRegistration_Filter();
}


void JrAutoAspect::gon_pluginModuleSingletonLoadDoRegistration_Filter() {
	// set up the global singleton info for this source filter type
	//
	setPluginCallbacks(&SingletonSourcePluginInfoJrAutoAspect_Filter);
	//
	SingletonSourcePluginInfoJrAutoAspect_Filter.id = PLUGIN_NAME_Filter;
	SingletonSourcePluginInfoJrAutoAspect_Filter.type = OBS_SOURCE_TYPE_FILTER;
	SingletonSourcePluginInfoJrAutoAspect_Filter.version = 1;
	SingletonSourcePluginInfoJrAutoAspect_Filter.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	SingletonSourcePluginInfoJrAutoAspect_Filter.icon_type = OBS_ICON_TYPE_COLOR;
	//
	obs_register_source(&SingletonSourcePluginInfoJrAutoAspect_Filter);
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
obs_properties_t* JrAutoAspect::gon_plugin_get_properties() {
	obs_properties_t *props = obs_properties_create();
	obs_properties_t* propgroup;

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "output", "Output size", OBS_GROUP_NORMAL , propgroup);

	obs_property_t* comboStringStyle = obs_properties_add_list(propgroup, "orientation", "Orientation", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboStringStyle, (const char**)JrOrientationMode_Choices);

	obs_properties_add_int_slider(propgroup, "maxWidth", "Maximum width", 0, 7680, 1);
	obs_properties_add_int_slider(propgroup, "maxHeight", "Maximum height", 0, 4320, 1);
	obs_properties_add_int_slider(propgroup, "recheckEveryMs", "Recheck frequency (ms; default 500)", 1, 5000, 1);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "portrait", "Portrait mode", OBS_GROUP_NORMAL , propgroup);

	obs_properties_add_int_slider(propgroup, "cropLeftp", "Crop left", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropRightp", "Crop right", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropTopp", "Crop top", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropBottomp", "Crop bottom", 0, 2000, 1);
		
	propgroup = obs_properties_create();
	obs_properties_add_group(props, "landscape", "Landscape mode", OBS_GROUP_NORMAL , propgroup);

	obs_properties_add_int_slider(propgroup, "cropLeftl", "Crop left", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropRightl", "Crop right", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropTopl", "Crop top", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropBottoml", "Crop bottom", 0, 2000, 1);

	return props;
}



void JrAutoAspect::gon_plugin_get_defaults(obs_data_t* settings) {
	obs_data_set_default_int(settings, "orientation", JrOrientationModeEnum_Auto);
	obs_data_set_default_int(settings, "recheckEveryMs", 500);
	
	obs_data_set_default_int(settings, "cropLeftp", 0);
	obs_data_set_default_int(settings, "cropRightp", 0);
	obs_data_set_default_int(settings, "cropTopp", 0);
	obs_data_set_default_int(settings, "cropBottomp", 0);
	obs_data_set_default_int(settings, "cropLeftl", 0);
	obs_data_set_default_int(settings, "cropRightl", 0);
	obs_data_set_default_int(settings, "cropTopl", 0);
	obs_data_set_default_int(settings, "cropBottoml", 0);
	
	obs_data_set_default_int(settings, "cropBottom", 0);
	obs_data_set_default_int(settings, "scale", 0);
	obs_data_set_default_int(settings, "panX", 0);
	obs_data_set_default_int(settings, "panY", 0);
	obs_data_set_default_int(settings, "aspectRatio", 0);

	obs_data_set_default_int(settings, "maxWidth", 960);
	obs_data_set_default_int(settings, "maxHeight", 580);
}




void JrAutoAspect::gon_plugin_update(obs_data_t* settings) {
	opt_orientationMode = (JrOrientationMode)jrPropertListChoiceFind(obs_data_get_string(settings, "orientation"), (const char**)JrOrientationMode_Choices, 0);
	//
	opt_cropLeftp = obs_data_get_int(settings, "cropLeftp");
	opt_cropRightp = obs_data_get_int(settings, "cropRightp");
	opt_cropTopp = obs_data_get_int(settings, "cropTopp");
	opt_cropBottomp = obs_data_get_int(settings, "cropBottomp");
	//
	opt_cropLeftl = obs_data_get_int(settings, "cropLeftl");
	opt_cropRightl = obs_data_get_int(settings, "cropRightl");
	opt_cropTopl = obs_data_get_int(settings, "cropTopl");
	opt_cropBottoml = obs_data_get_int(settings, "cropBottoml");
	//
	opt_maxWidth = obs_data_get_int(settings, "maxWidth");
	opt_maxHeight = obs_data_get_int(settings, "maxHeight");

	opt_recheckEveryMs = obs_data_get_int(settings, "recheckEveryMs");

	// and now make changes based on options changing
	forceUpdatePluginSettingsOnOptionChange();
}
//---------------------------------------------------------------------------


























//---------------------------------------------------------------------------
JrAutoAspect::JrAutoAspect() : jrObsPluginSource() {
	bool success = init();
	if (!success) {
		mydebug("Failed to init.");
		return;
	}
}


JrAutoAspect::~JrAutoAspect() {
	deInitialize();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrAutoAspect::onModulePostLoad() {

}

void JrAutoAspect::onModuleUnload() {

}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
bool JrAutoAspect::init() {
	// base class stuff
	initialStartup();


	// stuff we add
	bool success = true;
	if (true) {
		obs_enter_graphics();
		// does this really need to be done in graphics context?
		success &= loadEffects();
		obs_leave_graphics();
	}

	// reserver texrenders
	texrenderSource = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	texrenderTemp = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	texrenderStaging = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	// create helper mutex
	pluginMutex = CreateMutexA(NULL, FALSE, NULL);

	return success;
}



void JrAutoAspect::deInitialize() {

	//mydebug("IN JrAutoAspect Deinit.");

	if (true) {
		obs_enter_graphics();
		// free stuff in graphics mode?
		if (effectCropScale) {
			gs_effect_destroy(effectCropScale);
			effectCropScale = NULL;
		}

		//
		if (texrenderSource) {
			gs_texrender_destroy(texrenderSource);
			texrenderSource = NULL;
		}
		if (texrenderTemp) {
			gs_texrender_destroy(texrenderTemp);
			texrenderTemp = NULL;
		}
		if (texrenderStaging) {
			gs_texrender_destroy(texrenderStaging);
			texrenderStaging = NULL;
		}
		if (stagingSurface) {
			gs_stagesurface_destroy(stagingSurface);
			stagingSurface = NULL;
		}

		freeStagingSurfaceAndData();

		obs_leave_graphics();
	}

	if (pluginMutex) {
		ReleaseMutex(pluginMutex);
		CloseHandle(pluginMutex);
	}

	// base class stuff
	finalShutdown();
}
//---------------------------------------------------------------------------






















//---------------------------------------------------------------------------
bool JrAutoAspect::loadEffects() {
	char *effectPath = obs_module_file("jrAutoAspect_cropScale.effect");
	effectCropScale = gs_effect_create_from_file(effectPath, NULL);
	bfree(effectPath);
	if (!effectCropScale) {
		mydebug("ERROR loading effectCropScale effect.");
		return false;
	}

	// crop
	param_crop_mulVal = gs_effect_get_param_by_name(effectCropScale, "mulVal");
	param_crop_addVal = gs_effect_get_param_by_name(effectCropScale, "addVal");

	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool JrAutoAspect::gon_plugin_render(gs_effect_t* obsoleteFilterEffect) {
	return doRenderPluginFilter();
}
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
void JrAutoAspect::registerCallbacksAndHotkeys() {
	jrObsPlugin::registerCallbacksAndHotkeys();
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
}

void JrAutoAspect::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
}

// statics just reroute to a cast object member function call

void JrAutoAspect::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	JrAutoAspect *pluginp = reinterpret_cast<JrAutoAspect *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}


void JrAutoAspect::handleObsFrontendEvent(enum obs_frontend_event event) {
	switch ((int)event) {
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			// ATTN: todo mark as needing recheck
			setNeedsRecalculation(true);
			break;
		// ATTN:TODO other events that would be worth setting recheck?
	}

	// let parent handle some cases
	jrObsPluginSource::handleObsFrontendEvent(event);
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
bool JrAutoAspect::doRenderPluginFilter() {
	// this is called for FILTER type plugin
	// calc settings for scene
	updateSourceProperties();

	if (sourceWidth != cachedSourceWidth || sourceHeight != cachedSourceHeight) {
		setNeedsRecalculation(true);
	}

	// force periodic recheck
	unsigned long thisTimeMs = jr_os_gettime_ms();
	if (thisTimeMs - lastCheckTimeMs > opt_recheckEveryMs) {
		// force periodic recheck
		setNeedsRecalculation(true);
	}

	if (flagNeedsRecalculation) {
		// recalculate
		recalculateAspectRatioChoice();
		// track this so we can detect when it changes
		cachedSourceWidth = sourceWidth;
		cachedSourceHeight = sourceHeight;
	}

	// are we engaged and runable?
	if (orientationMode == JrOrientationModeEnum_Auto) {
		// was not determined
		return false;
	}
	if (!effectCropScale || !renderSource || sourceWidth <= 10 || sourceHeight <= 10 || renderedWidth<=10 || renderedHeight<=10) {
		// dont render normal source, better to show there is error with blankness
		return false;
	}

	// If we try rendering directly to screen it blanks out the entire background unless we have another filter afterwards; dont really understand why..
	bool flag_renderToTempForCompositing = true;
	gs_texrender_t* outTexrender = (flag_renderToTempForCompositing) ? texrenderTemp : NULL;

	// draw source into holding texture first with default effect
	jrRenderSourceIntoTexture(renderSource, texrenderSource, sourceWidth, sourceHeight, jrBlendClearOverwite);

	// ok render the source to output texture (or display) at desired crop/size/position
	doRenderEffectCropScale(outTexrender, texrenderSource, sourceWidth, sourceHeight);
	//doRenderEffectCropScale(outTexrender, texrenderSource, renderedWidth, renderedHeight);

	// if we rendered to temp then now to screen
	if (outTexrender!=NULL) {
		// from temp to screen
		jrRenderTextureIntoTexture(NULL, outTexrender, renderedWidth, renderedHeight, jrBlendSrcObsSep);
	}

	// actual size
	width = renderedWidth;
	height = renderedHeight;

	return true;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void JrAutoAspect::doRenderEffectCropScale(gs_texrender_t* texDest, gs_texrender_t* texSource, int swidth, int sheight) {
	// note that this used PRECOMPUTED values so that we only have to recompute on changed size or options
	doSetEffectParamsCropScale();
	//jrRenderEffectIntoTextureAtSizeLoc(texDest, effectCropScale, texSource, NULL, swidth, sheight, 0, 0, renderedWidth, renderedHeight, jrBlendPureCopy, "Draw", width, height);
	jrRenderEffectIntoTextureAtSizeLoc(texDest, effectCropScale, texSource, NULL, swidth, sheight, 0, 0, renderedWidth, renderedHeight, jrBlendPureCopy, "Draw", renderedWidth, renderedHeight);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrAutoAspect::doSetEffectParamsCropScale() {
	gs_effect_set_vec2(param_crop_addVal, &cropAddVal);
	gs_effect_set_vec2(param_crop_mulVal, &cropMulVal);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrAutoAspect::recalculateAspectRatioChoice() {
	// orientation can be manually set or set to auto
	JrOrientationMode newOrientationMode = opt_orientationMode;
	if (opt_orientationMode == JrOrientationModeEnum_Auto) {
		// guess it
		 newOrientationMode = guessOrientation();
	}
	if (orientationMode != newOrientationMode) {
		// it changes
		orientationMode = newOrientationMode;
		//mydebug("Changing orientation mode to %d", newOrientationMode);
	}

	// update calculations based on new orientation
	setCalculationsBasedOnOrientation();

	// clear this
	setNeedsRecalculation(false);
	lastCheckTimeMs = jr_os_gettime_ms();
}





void JrAutoAspect::setCalculationsBasedOnOrientation() {
	float cropLeft, cropRight, cropTop, cropBottom;
	if (orientationMode == JrOrientationModeEnum_Portrait) {
		cropLeft = opt_cropLeftp;
		cropTop = opt_cropTopp;
		cropRight = opt_cropRightp;
		cropBottom = opt_cropBottomp;
	} else if (orientationMode == JrOrientationModeEnum_Landscape){
		cropLeft = opt_cropLeftl;
		cropTop = opt_cropTopl;
		cropRight = opt_cropRightl;
		cropBottom = opt_cropBottoml;
	}
	else {
		cropLeft = 0;
		cropTop = 0;
		cropRight = 0;
		cropBottom = 0;
	}

	// cropped size before scaling
	float sourceWidthCropped = sourceWidth - (cropLeft + cropRight);
	float sourceHeightCropped = sourceHeight - (cropTop + cropBottom);

	// sanity check
	if (sourceWidthCropped <= 2 || sourceHeightCropped <= 2) {
		return;
	}

	// now autoscale to fit in max width and height
	float xscale = 1.0;
	float yscale = 1.0;
	//
	if (opt_maxWidth > 0) {
		xscale = (float)opt_maxWidth / sourceWidthCropped;
	}

	if (opt_maxHeight > 0) {
		yscale = (float)opt_maxHeight / sourceHeightCropped;
	}

	// overall scale, constrained to most constraining size
	float scale;
	if (xscale > yscale || opt_maxWidth == 0) {
		scale = yscale;
	}
	else {
		scale = xscale;
	}

	// set values
	renderedWidth = sourceWidthCropped;
	renderedHeight = sourceHeightCropped;
	//
	cropAddVal.x=(float)cropLeft / (float)sourceWidth;
	cropAddVal.y=(float)cropTop / (float)sourceHeight;
	cropMulVal.x=renderedWidth / (float)sourceWidth;
	cropMulVal.y=renderedHeight / (float)sourceHeight;

	// scaling of final output
	renderedWidth *= scale;
	renderedHeight *= scale;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
JrOrientationMode JrAutoAspect::guessOrientation() {

	// sanity check
	if (sourceWidth <= 10 || sourceHeight <= 10) {
		// return saying couldnt decide?
		//return JrOrientationModeEnum_Auto;
		//mydebug("guessOrientation 1");
		return JrOrientationModeEnum_Landscape;
	}

	//mydebug("guessOrientation 2");

	// ok so the workflow is as follows:
	// IF we see ONLY black on the SIDES, then we assume portrait mode, otherwise landscape
	// to be efficient, we will not check every row or pixel, and not try to determine the exact crop
	// we will look at one column on left and one on right, half way between edge and crop
	// conceivably we could make this an option but it seems reasonable
	// scale down texture we are going to look at, for efficiency? is this really more efficient than just checking pixels in a full size texture?
	int resizex, resizey;
	int rowEvery;
	if (true) {
		resizex = DefResizeFactorForScanningX;
		resizey = DefResizeFactorForScanningY;
		rowEvery = 1;
	}
	else {
		// scan on original
		resizex = 1;
		resizey = 1;
		rowEvery = DefResizeFactorForScanningY;
	}
	int stageWidth = sourceWidth / resizex;
	int stageHeight = sourceHeight / resizey;
	//
	int lookxmin = (opt_cropLeftp / 2) / resizex;
	int lookxmax = (sourceWidth - (opt_cropRightp / 2)) / resizex;

	// sanity check
	if (lookxmin < 0 || lookxmax >= stageWidth || stageWidth <= 2 || stageHeight <= 2) {
		return JrOrientationModeEnum_Landscape;
	}

	// ok now we need the source in a way we can examine pixels
	//mydebug("guessOrientation 3 %dx%d",stageWidth, stageHeight);

	// create spaces and allocate
	// realloc internal memory
	reallocateStagingSurfaceAndData(stageWidth, stageHeight);

	// render source into staging texture
	jrRenderSourceIntoTextureAtSizeLoc(renderSource, texrenderStaging, sourceWidth, sourceHeight, 0, 0, stageWidth, stageHeight, jrBlendClearOverwite, false);

	// now prepare to get access to this rendered stage texture into internal ram
	gs_texture_t *tex = gs_texrender_get_texture(texrenderStaging);
	if (!tex) {
		mydebug("Failed to create tex from %dx%d to  %dx%d",sourceWidth, sourceHeight, stageWidth, stageHeight);
		return JrOrientationModeEnum_Landscape;
	}

	// stage tex in staging surface
	gs_stage_texture(stagingSurface, tex);

	uint8_t *idata;
	uint32_t ilinesize;
	bool stageMemoryReady = false;

	WaitForSingleObject(pluginMutex, INFINITE);
	if (gs_stagesurface_map(stagingSurface, &idata, &ilinesize)) {
		memcpy(stagingData, idata, ilinesize * stageHeight);
		// update our stored data linesize
		stagingLinesize = ilinesize;
		gs_stagesurface_unmap(stagingSurface);
		stageMemoryReady = true;
	} else {
		mydebug("ERROR ------> JrAutoAspect::guessOrientation failed in call to gs_stagesurface_map.");
	}
	ReleaseMutex(pluginMutex);

	if (!stageMemoryReady) {
		//mydebug("guessOrientation 5 - stageMemoryReady false");
		return JrOrientationModeEnum_Landscape;
	}

	// ok its in ram, now walk it and check pixels
	uint32_t pixel1, pixel2;
	bool flagFoundNonBlankContent = false;
	for (int y = 0; y < stageHeight; y += rowEvery) {
		pixel1 = getPixelFromStagingData((uint32_t*)stagingData, lookxmin, y, stagingLinesize);
		pixel2 = getPixelFromStagingData((uint32_t*)stagingData, lookxmax, y, stagingLinesize);
		if (!isPixelBlank(pixel1) || !isPixelBlank(pixel2)) {
			//mydebug("Examining pixel at %d,%d = %#08x", lookxmin, y, pixel1);
			//mydebug("Examining pixel at %d,%d = %#08x", lookxmax, y, pixel2);
			flagFoundNonBlankContent = true;
			break;
		}
	}


	if (flagFoundNonBlankContent) {
		//mydebug("guessOrientation 6");
		return JrOrientationModeEnum_Landscape;
	}

	// portrait!
	//mydebug("guessOrientation 7");
	return JrOrientationModeEnum_Portrait;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrAutoAspect::reallocateStagingSurfaceAndData(int stageWidth, int stageHeight) {
	if (stagingData != 0 && stageWidth == allocatedStageWidth && stageHeight == allocatedStageHeight) {
		// nothing changed, nothing to do
		return;
	}
	// free previous
	freeStagingSurfaceAndData();
	// allocate new
	stagingSurface = gs_stagesurface_create(stageWidth, stageHeight, GS_RGBA);
	stagingData = (uint8_t *) bzalloc((stageWidth + 32) * stageHeight * 4);
	// remember how big
	allocatedStageWidth = stageWidth;
	allocatedStageHeight = stageHeight;
}

void JrAutoAspect::freeStagingSurfaceAndData() {
	if (stagingData) {
		bfree(stagingData);
		stagingData = NULL;
	}
	if (stagingSurface) {
		gs_stagesurface_destroy(stagingSurface);
		stagingSurface = NULL;
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
uint32_t JrAutoAspect::getPixelFromStagingData(uint32_t* data, int x, int y, int dataLineSize) {
	int pointOffset = CompPointOffset(x, y, dataLineSize/4);
	return data[pointOffset];
}


bool JrAutoAspect::isPixelBlank(uint32_t pixel) {
//	return ((pixel == DefUint32PixelColorBlank1) || (pixel == DefUint32PixelColorBlank2) || (pixel == DefUint32PixelColorBlank3));
	char* bytep = (char*) &pixel;
	unsigned char r = *bytep++;
	unsigned char g = *bytep++;
	unsigned char b = *bytep++;
	unsigned char a = *bytep++;
	const int maxVal = 20;
	if (r < maxVal && g < maxVal && b < maxVal && r == g && r == b) {
		//mydebug("BLANK: r=%d g=%d b=%d  a=%d", r, g, b, a);
		return true;
	}
	//mydebug("CONTENT: r=%d g=%d b=%d  a=%d", r, g, b, a);
	return false;
}
//---------------------------------------------------------------------------
