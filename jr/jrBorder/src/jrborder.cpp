/*
ObsScrenFlip - flips left and right halves of screen (configurable where) so that when you read from preview screen
it looks like you are looking off in the other direction.
*/

#include "pluginInfo.hpp"
//
#include "jrborder.hpp"

// this MUST come after pluginInfo included, as it depends on macros there
#include "../../jrcommon/src/jrobsplugin_source_globalfuncs.hpp"

//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>

#include <string>
#include <vector>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>





//---------------------------------------------------------------------------
const char* Setting_borderStyle_Choices[] = { "outer", "innerShrink", "innerOverlap", NULL };

//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
// global singleton source info
struct obs_source_info SingletonSourcePluginInfoJrBorder;
//
void JrBorder::gon_pluginModuleSingletonLoadDoRegistration() {
	// set up the global singleton info for this source filter type
	//
	setPluginCallbacks(&SingletonSourcePluginInfoJrBorder);
	//
	SingletonSourcePluginInfoJrBorder.id = PLUGIN_NAME_Filter;
	SingletonSourcePluginInfoJrBorder.type = OBS_SOURCE_TYPE_FILTER;
	SingletonSourcePluginInfoJrBorder.version = 1;
	SingletonSourcePluginInfoJrBorder.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	SingletonSourcePluginInfoJrBorder.icon_type = OBS_ICON_TYPE_COLOR;
	//
	obs_register_source(&SingletonSourcePluginInfoJrBorder);
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
JrBorder::JrBorder() : jrObsPluginSource() {
	bool success = init();
	if (!success) {
		mydebug("Failed to init.");
		return;
	}
}


JrBorder::~JrBorder() {
	deInitialize();
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrBorder::onModulePostLoad() {

}

void JrBorder::onModuleUnload() {

}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
bool JrBorder::init() {
	sourceWidth = NULL;
	renderSource = NULL;

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
	texrenderPreBorder = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	return success;
}



void JrBorder::deInitialize() {
	if (true) {
		obs_enter_graphics();
		// free stuff in graphics mode?
		if (effectCropScale) {
			gs_effect_destroy(effectCropScale);
			effectCropScale = NULL;
		}
		if (effectBorder) {
			gs_effect_destroy(effectBorder);
			effectBorder = NULL;
		}
		if (texrenderSource) {
			gs_texrender_destroy(texrenderSource);
			texrenderSource = NULL;
		}
		if (texrenderPreBorder) {
			gs_texrender_destroy(texrenderPreBorder);
			texrenderPreBorder = NULL;
		}

		obs_leave_graphics();
	}

	// base class stuff
	finalShutdown();
}
//---------------------------------------------------------------------------






















//---------------------------------------------------------------------------
bool JrBorder::loadEffects() {
	char *effectPath = obs_module_file("jrborder_cropScale.effect");
	effectCropScale = gs_effect_create_from_file(effectPath, NULL);
	bfree(effectPath);
	if (!effectCropScale) {
		mydebug("ERROR loading effectCropScale effect.");
		return false;
	}

	effectPath = obs_module_file("jrborder_border.effect");
	effectBorder = gs_effect_create_from_file(effectPath, NULL);
	bfree(effectPath);
	if (!effectBorder) {
		mydebug("ERROR loading effectBorder effect.");
		return false;
	}

	// crop
	param_crop_mulVal = gs_effect_get_param_by_name(effectCropScale, "mulVal");
	param_crop_addVal = gs_effect_get_param_by_name(effectCropScale, "addVal");

	// border
	param_border_src_minx = gs_effect_get_param_by_name(effectBorder, "border_src_minx");
	param_border_src_miny = gs_effect_get_param_by_name(effectBorder, "border_src_miny");
	param_border_src_maxx = gs_effect_get_param_by_name(effectBorder, "border_src_maxx");
	param_border_src_maxy = gs_effect_get_param_by_name(effectBorder, "border_src_maxy");
	param_border_color = gs_effect_get_param_by_name(effectBorder, "border_color");
	param_border_mulVal = gs_effect_get_param_by_name(effectBorder, "mulVal");
	param_border_addVal = gs_effect_get_param_by_name(effectBorder, "addVal");

	return true;
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
obs_properties_t* JrBorder::gon_plugin_get_properties() {
	obs_properties_t *props = obs_properties_create();
	obs_properties_t* propgroup;

	
	propgroup = obs_properties_create();
	obs_properties_add_group(props, "border", "Enable border", OBS_GROUP_CHECKABLE , propgroup);
	//obs_properties_add_bool(propgroup, Setting_EnabledBorder, Setting_EnabledBorder_Text);

	obs_property_t* comboStringStyle = obs_properties_add_list(propgroup, Setting_borderStyle, Setting_borderStyle_Text, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboStringStyle, (const char**)Setting_borderStyle_Choices);

	obs_properties_add_int_slider(propgroup, Setting_borderWidth, Setting_borderWidth_Text,1,255,1);
	obs_properties_add_color_alpha(propgroup, Setting_borderColor, Setting_borderColor_Text);


	propgroup = obs_properties_create();
	obs_properties_add_group(props, "cropzoom", "Enable crop and scal", OBS_GROUP_CHECKABLE , propgroup);
	//obs_properties_add_bool(propgroup, Setting_EnabledCropZoom, Setting_EnabledCropZoom_Text);
	obs_properties_add_int_slider(propgroup, "cropLeft", "Crop left", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropRight", "Crop right", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropTop", "Crop top", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "cropBottom", "Crop bottom", 0, 2000, 1);
	obs_properties_add_int_slider(propgroup, "scale", "Scale", -1000, 1000, 1);
	//
	propgroup = obs_properties_create();
	obs_properties_add_group(props, "tweaks", "Tweaks", OBS_GROUP_NORMAL , propgroup);
	obs_properties_add_int_slider(propgroup, "panX", "Pan X", -1000, 1000, 1);
	obs_properties_add_int_slider(propgroup, "panY", "Pan Y", -1000, 1000, 1);
	obs_properties_add_int_slider(propgroup, "aspectRatio", "Aspect ratio", -1000, 1000, 1);
	return props;
}


void JrBorder::gon_plugin_get_defaults(obs_data_t* settings) {
	obs_data_set_default_bool(settings, "border", true);
	obs_data_set_default_bool(settings, "cropzoom", true);
	obs_data_set_default_bool(settings, Setting_EnabledBorder, Setting_EnabledBorder_Def);
	obs_data_set_default_string(settings, Setting_borderStyle, Setting_borderStyle_Text);
	obs_data_set_default_int(settings, Setting_borderWidth, Setting_borderWidth_Def);
	obs_data_set_default_int(settings, Setting_borderColor, Setting_borderColor_Def);
	//
	obs_data_set_default_bool(settings, Setting_EnabledCropZoom, Setting_EnabledCropZoom_Def);
	obs_data_set_default_int(settings, "cropLeft", 0);
	obs_data_set_default_int(settings, "cropRight", 0);
	obs_data_set_default_int(settings, "cropTop", 0);
	obs_data_set_default_int(settings, "cropBottom", 0);
	obs_data_set_default_int(settings, "cropBottom", 0);
	//
	obs_data_set_default_int(settings, "scale", 0);
	//
	obs_data_set_default_int(settings, "panX", 0);
	obs_data_set_default_int(settings, "panY", 0);
	obs_data_set_default_int(settings, "aspectRatio", 0);
}


void JrBorder::gon_plugin_update(obs_data_t* settings) {
	// get options
	//opt_enable_border = obs_data_get_bool(settings, Setting_EnabledBorder);
	opt_enable_border = obs_data_get_bool(settings, "border");
	opt_borderStyle = jrPropertListChoiceFind(obs_data_get_string(settings, Setting_borderStyle), (const char**)Setting_borderStyle_Choices,0);
	opt_borderWidth = obs_data_get_int(settings, Setting_borderWidth);
	opt_borderColor = obs_data_get_int(settings, Setting_borderColor);
	//
	//opt_enable_cropzoom = obs_data_get_bool(settings, Setting_EnabledCropZoom);
	opt_enable_cropzoom = obs_data_get_bool(settings, "cropzoom");
	opt_cropLeft = obs_data_get_int(settings, "cropLeft");
	opt_cropRight = obs_data_get_int(settings, "cropRight");
	opt_cropTop = obs_data_get_int(settings, "cropTop");
	opt_cropBottom = obs_data_get_int(settings, "cropBottom");
	//
	opt_scale = obs_data_get_int(settings, "scale");
	//
	opt_panX = obs_data_get_int(settings, "panX");
	opt_panY = obs_data_get_int(settings, "panY");
	//
	opt_aspectRatio = obs_data_get_int(settings, "aspectRatio");

	// pan really just adjusts crop
	opt_cropLeft += opt_panX;
	opt_cropRight -= opt_panX;
	opt_cropTop += opt_panY;
	opt_cropBottom -= opt_panY;
	// cap
	opt_cropLeft = max(opt_cropLeft, 0.0);
	opt_cropRight = max(opt_cropRight, 0.0);
	opt_cropTop = max(opt_cropTop, 0.0);
	opt_cropBottom = max(opt_cropBottom, 0.0);

	// and now make changes based on options changing
	forceUpdatePluginSettingsOnOptionChange();
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
bool JrBorder::gon_plugin_render(gs_effect_t* obsoleteFilterEffect) {
	// calc settings for scene
	updateSourceProperties();


	if (!effectBorder || !effectCropScale || !renderSource || sourceWidth == 0) {
		// dont render normal source, better to show there is error with blankness
		return false;
	}

	if (!isEnabledBorder() && !isEnabledCropOrZoom()) {
		// nothing to do
		if (true) {
			obs_source_skip_video_filter(context);
		}
		else {
			jrRenderSourceOut(renderSource, sourceWidth, sourceHeight, sourceWidth, sourceHeight, jrBlendSrcAlphaMerge);
		}
		width = sourceWidth;
		height = sourceHeight;
		return true;
	}


	// draw source into holding texture first with default effect
	jrRenderSourceIntoTexture(renderSource, texrenderSource, sourceWidth, sourceHeight, jrBlendClearOverwite);

	// starting texture size
	int renderedWidth = sourceWidth;
	int renderedHeight = sourceHeight;
	gs_texrender_t* nextTextureIn = texrenderSource;
	gs_texrender_t* nextTextureOut = NULL;

	if (isEnabledCropOrZoom()) {
		// do crop scale rendering
		if (isEnabledBorder()) {
			// out to temp buffer
			nextTextureOut = texrenderPreBorder;
		}
		else {
			// out to screen
			nextTextureOut = NULL;
		}
		doRenderEffectCropScale(nextTextureIn, nextTextureOut, renderedWidth, renderedHeight);
		nextTextureIn = nextTextureOut;
		nextTextureOut = NULL;
	}
	else {
		// temp source is input, NULL display is output
		nextTextureIn = texrenderSource;
		nextTextureOut = NULL;
	}

	if (isEnabledBorder()) {
		// add border and render to output
		doRenderEffectBorder(nextTextureIn, nextTextureOut, renderedWidth, renderedHeight);
		nextTextureIn = nextTextureOut;
		nextTextureOut = NULL;
	}

	// fininished size
	width = renderedWidth;
	height = renderedHeight;

	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrBorder::doRenderEffectCropScale(gs_texrender_t* texSource, gs_texrender_t* texDest, int &renderedWidth, int &renderedHeight) {
	// for crop scale we first can apply a crop to the source image in pixels
	// then we scale the image up or down

	// remember source size
	int swidth = renderedWidth;
	int sheight = renderedHeight;

	// getarget dimensions for crop, before scale
	renderedWidth = swidth - (opt_cropLeft + opt_cropRight);
	renderedHeight = sheight - (opt_cropTop + opt_cropBottom);

	struct vec2 offVal { 0 };
	offVal.x = (float)opt_cropLeft/(float)swidth;
	offVal.y = (float)opt_cropTop / (float)sheight;
	gs_effect_set_vec2(param_crop_addVal, &offVal);	

	// crop adjustment
	struct vec2 mulVal { 0 };
	mulVal.x = (float) renderedWidth / (float)swidth;
	mulVal.y = (float) renderedHeight / (float)sheight;
	gs_effect_set_vec2(param_crop_mulVal, &mulVal);

	// scaling mod on top
	if (opt_scale != 0 || opt_aspectRatio!=0) {
		// convert - + opt_scale to shrink vs enlarge multiplier
		float scaleMult;
		if (opt_scale < 0) {
			scaleMult = 1.0f / (1.0f+(float)opt_scale/-100.0f);
		}
		else {
			scaleMult = 1.0f + ((float)opt_scale/100.0f);
		}
		// x and y scale start same
		float scaleMultX = scaleMult;
		float scaleMultY = scaleMult;
		// aspect ratio modifiers
		if (opt_aspectRatio == 0) {
			// nothing to do
		} else if (opt_aspectRatio < 0) {
			scaleMultY *= 1.0f / (1.0f + (float)opt_aspectRatio / -100.0f);
		}
		else {
			scaleMultX *= 1.0f / (1.0f + (float)opt_aspectRatio / 100.0f);
		}
		// 
		renderedWidth *= scaleMultX;
		renderedHeight *= scaleMultY;
	}

	if (calcUseScaleStageForThisShrinkStyle()) {
		// this is a kludge to have US to the resize to convert a shrink mode border to an outer border to avoid scaling twice
		float scaleMultX = (float)renderedWidth / (float)(renderedWidth + opt_borderWidth * 2);
		float scaleMultY = (float)renderedHeight / (float)(renderedHeight + opt_borderWidth * 2);
		renderedWidth *= scaleMultX;
		renderedHeight *= scaleMultY;
	}

	jrRenderEffectIntoTextureAtSizeLoc(texDest, effectCropScale, texSource, NULL, swidth, sheight, 0, 0, renderedWidth, renderedHeight, jrBlendSrcAlphaMerge, "Draw");
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrBorder::doRenderEffectBorder(gs_texrender_t* texSource, gs_texrender_t* texDest, int &renderedWidth, int &renderedHeight) {
	// remember source size
	int swidth = renderedWidth;
	int sheight = renderedHeight;
	float mulx, muly;
	float offx, offy;
	float borderDivX = 1.0f;
	float borderDivY = 1.0f;

	// style of border
	if (opt_borderStyle == borderStyleEnumOuter || calcUseScaleStageForThisShrinkStyle()) {
		// filter becomes BIGGER to put a border AROUND the untouched source
		renderedWidth = swidth + opt_borderWidth * 2;
		renderedHeight = sheight + opt_borderWidth * 2;
		//
		// multiplication values are set so that when we ENLARGE the projection, the actual source image does NOT change size (confusing i know)
		mulx = (float)renderedWidth / (float)swidth;
		muly = (float)renderedHeight / (float)sheight;
		offx = -(float)opt_borderWidth / (float)swidth;
		offy = -(float)opt_borderWidth / (float)sheight;
	} else if (opt_borderStyle == borderStyleEnumInnerShrink) {
		// filter same size as original but original shrinks
		// so final filter size stays same, but we SHRINK image so it fits in new borders
		float shrunkSourceWidth = swidth - opt_borderWidth * 2;
		float shrunkSourceHeight = sheight - opt_borderWidth * 2;
		// calculate mul for this
		mulx = (float)swidth / (float)shrunkSourceWidth;
		muly = (float)sheight / (float)shrunkSourceHeight;
		offx = -(float)opt_borderWidth / (float)shrunkSourceWidth;
		offy = -(float)opt_borderWidth / (float)shrunkSourceHeight;
		borderDivX = mulx;
		borderDivY = muly;
	} else if (opt_borderStyle == borderStyleEnumInnerOverlap) {
		// filter same size, source same size, border on top of source
		mulx = 1.0f;
		muly = 1.0f;
		offx = 0.0f;
		offy = 0.0f;
	}

	// set mul and off
	struct vec2 offVal {offx, offy };
	gs_effect_set_vec2(param_border_addVal, &offVal);	
	struct vec2 mulVal { mulx, muly };
	gs_effect_set_vec2(param_border_mulVal, &mulVal);

	// set color
	struct vec4 borderColorVec;
	jrazUint32ToRgbaVec(opt_borderColor, borderColorVec);
	gs_effect_set_vec4(param_border_color, &borderColorVec);

	// border
	// border is defined in the 0-1 scale of the source
	float border_src_minx, border_src_miny, border_src_maxx, border_src_maxy;
	border_src_minx = (float)opt_borderWidth / (float)swidth;
	border_src_miny = (float)opt_borderWidth / (float)sheight;
	border_src_maxx = 1.0f - border_src_minx;
	border_src_maxy = 1.0f - border_src_miny;

	// fixup for scaling offset
	border_src_minx += offx / borderDivX;
	border_src_maxx -= offx / borderDivX;
	border_src_miny += offy / borderDivY;
	border_src_maxy -= offy / borderDivY;


	// set it
	gs_effect_set_float(param_border_src_minx, border_src_minx);
	gs_effect_set_float(param_border_src_miny, border_src_miny);
	gs_effect_set_float(param_border_src_maxx, border_src_maxx);
	gs_effect_set_float(param_border_src_maxy, border_src_maxy);

	jrRenderEffectIntoTextureAtSizeLoc(texDest, effectBorder, texSource, NULL, swidth, sheight, 0, 0, renderedWidth, renderedHeight, jrBlendSrcAlphaMerge, "Draw");
}
//---------------------------------------------------------------------------



