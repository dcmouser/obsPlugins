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



//---------------------------------------------------------------------------
#define DefJrAvoidAlphaMaskDrawingUnnescesarily true
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
const char* Setting_borderStyle_Choices[] = { "constrain", "grow", NULL };
const char* Setting_borderShape_Choices[] = { "rectangle", "circle", "custom mask file", NULL};
const char* Setting_customMaskType_Choices[] = { "black vs white", "2-bit alpha", "full color chroma", NULL};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// forward declarations that are extern "C"
#ifdef __cplusplus
extern "C" {
#endif
	bool OnPropertyButtonClickAdjustNowCallback(obs_properties_t *props, obs_property_t *property, void *data);
	bool OnPropertyChangeCallback(obs_properties_t* props, obs_property_t* p, obs_data_t* settings);
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
bool OnPropertyChangeCallback(obs_properties_t* props, obs_property_t* p, obs_data_t* settings) {
	UNUSED_PARAMETER(p);
	int borderShape = jrPropertListChoiceFind(obs_data_get_string(settings, "shape"), (const char**)Setting_borderShape_Choices, 0);
	int bordermaskType = (Setting_customMaskType_ChoicesEnum) jrPropertListChoiceFind(obs_data_get_string(settings, SettingCustomMaskType), (const char**)Setting_customMaskType_Choices,0);
	bool backTextureEnabled = obs_data_get_bool(settings, "backTexture");

	bool visCustomMaskGroup = false;
	bool visBorderSyntaxGroup = false;

	if (borderShape == borderShapeEnumRectangle) {
		visBorderSyntaxGroup = true;
	} else if (borderShape == borderShapeEnumCircle) {
		visBorderSyntaxGroup = true;
	} else if (borderShape == borderShapeEnumCustom) {
		visCustomMaskGroup = true;
	}

	obs_property_set_visible(obs_properties_get(props, "custommask"), visCustomMaskGroup);
	obs_property_set_visible(obs_properties_get(props, "customColorSyntax"), visBorderSyntaxGroup);

	if (true) {
		bool visShapeWarn = (borderShape != borderShapeEnumRectangle);
		obs_property_set_visible(obs_properties_get(props, "noteSourceShape"), visShapeWarn);
	}

	if (true) {
		bool visAlphaLerpMode = (bordermaskType == customMaskTypeEnumChromaAlphaLerp) && (borderShape == borderShapeEnumCustom);
		obs_property_set_visible(obs_properties_get(props, "colorBlendLerp"), visAlphaLerpMode);
		obs_property_set_visible(obs_properties_get(props, "borderoffset"), !visAlphaLerpMode);
		obs_property_set_visible(obs_properties_get(props, "maskInside"), !visAlphaLerpMode);
	}

	if (true) {
		bool vis = backTextureEnabled;
		obs_property_set_visible(obs_properties_get(props, "colorBlendLerpBack"), vis);
	}

	return true;
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// global singleton source info
struct obs_source_info SingletonSourcePluginInfoJrBorder_Source;
struct obs_source_info SingletonSourcePluginInfoJrBorder_Filter;
//
void JrBorder::gon_pluginModuleSingletonLoadDoRegistration() {
	gon_pluginModuleSingletonLoadDoRegistration_Source();
	gon_pluginModuleSingletonLoadDoRegistration_Filter();
}


void JrBorder::gon_pluginModuleSingletonLoadDoRegistration_Source() {
	// set up the global singleton info for this source filter type
	//
	setPluginCallbacks(&SingletonSourcePluginInfoJrBorder_Source);
	//
	SingletonSourcePluginInfoJrBorder_Source.id = PLUGIN_NAME_Source;
	SingletonSourcePluginInfoJrBorder_Source.type = OBS_SOURCE_TYPE_INPUT;
	SingletonSourcePluginInfoJrBorder_Source.version = 1;
	SingletonSourcePluginInfoJrBorder_Source.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_SRGB;
	SingletonSourcePluginInfoJrBorder_Source.icon_type = OBS_ICON_TYPE_COLOR;
	//
	obs_register_source(&SingletonSourcePluginInfoJrBorder_Source);
}

void JrBorder::gon_pluginModuleSingletonLoadDoRegistration_Filter() {
	// set up the global singleton info for this source filter type
	//
	setPluginCallbacks(&SingletonSourcePluginInfoJrBorder_Filter);
	//
	SingletonSourcePluginInfoJrBorder_Filter.id = PLUGIN_NAME_Filter;
	SingletonSourcePluginInfoJrBorder_Filter.type = OBS_SOURCE_TYPE_FILTER;
	SingletonSourcePluginInfoJrBorder_Filter.version = 1;
	SingletonSourcePluginInfoJrBorder_Filter.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	SingletonSourcePluginInfoJrBorder_Filter.icon_type = OBS_ICON_TYPE_COLOR;
	//
	obs_register_source(&SingletonSourcePluginInfoJrBorder_Filter);
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
obs_properties_t* JrBorder::gon_plugin_get_properties() {
	obs_properties_t *props = obs_properties_create();
	obs_properties_t* propgroup;

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "border", "Enable border", OBS_GROUP_CHECKABLE , propgroup);

	obs_property_t* comboStringStyle = obs_properties_add_list(propgroup, "shape", "Shape", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboStringStyle, (const char**)Setting_borderShape_Choices);
	obs_property_set_modified_callback(comboStringStyle, OnPropertyChangeCallback);
	obs_properties_add_bool(propgroup, "maskInside", "Mask contents inside outer boundary of shape");
	//
	if (getIsPluginTypeFilter()) {
		obs_property_t* comboStringStyle = obs_properties_add_list(propgroup, Setting_borderStyle, Setting_borderStyle_Text, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
		obs_property_set_modified_callback(comboStringStyle, OnPropertyChangeCallback);
		jrAddPropertListChoices(comboStringStyle, (const char**)Setting_borderStyle_Choices);
	}
	else {
		obs_properties_add_text(propgroup, "noteSourceShape", "NOTE: Use the filter version (not this source version) of JrBorder plugin if you need transparency of non-rectangle shapes.", OBS_TEXT_INFO);
	}
	//
	obs_properties_add_int_slider(propgroup, Setting_borderWidth, Setting_borderWidth_Text,0,255,1);
	obs_properties_add_int_slider(propgroup, "borderoffset", "Border inside offset % (default 0)", 0, 100, 1);
	obs_properties_add_color_alpha(propgroup, Setting_borderColor, Setting_borderColor_Text);
	obs_properties_add_int_slider(propgroup, "colorBlendLerp", "Color blend %", 0, 100, 1);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "custommask", "Custom mask file", OBS_GROUP_NORMAL , propgroup);

	if (true) {
		std::string filterStr = std::string(TEXT_PATH_IMAGES) + std::string(IMAGE_FILTER_EXTENSIONS ";;") + std::string(TEXT_PATH_ALL_FILES) + std::string(" (*.*)");
		std::string startFolderPath = jrGetDirPathFromFilePath(opt_customMaskFilePath);
		obs_properties_add_path(propgroup, "maskFilePath", "Mask file path", OBS_PATH_FILE, filterStr.c_str(), startFolderPath.c_str());
	}

	comboStringStyle = obs_properties_add_list(propgroup, SettingCustomMaskType, "Custom mask type", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_set_modified_callback(comboStringStyle, OnPropertyChangeCallback);
	jrAddPropertListChoices(comboStringStyle, (const char**)Setting_customMaskType_Choices);
	obs_properties_add_bool(propgroup, "aspectStretchCustomMask", "Stretch mask (discard aspect ratio");

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "customColorSyntax", "Custom border pattern", OBS_GROUP_CHECKABLE , propgroup);
	//
	obs_properties_add_text(propgroup, "borderformat", "Custom border format (#,#,...)", OBS_TEXT_DEFAULT);
	obs_properties_add_color_alpha(propgroup, "color2", "2nd border color");
	obs_properties_add_color_alpha(propgroup, "color3", "3rd border color");
	obs_properties_add_color_alpha(propgroup, "color4", "4th border color");

	if (getIsPluginTypeFilter()) {
		propgroup = obs_properties_create();
		obs_properties_add_group(props, "cropzoom", "Enable crop and scale", OBS_GROUP_CHECKABLE, propgroup);
		//obs_properties_add_bool(propgroup, Setting_EnabledCropZoom, Setting_EnabledCropZoom_Text);
		obs_properties_add_int_slider(propgroup, "cropLeft", "Crop left", 0, 2000, 1);
		obs_properties_add_int_slider(propgroup, "cropRight", "Crop right", 0, 2000, 1);
		obs_properties_add_int_slider(propgroup, "cropTop", "Crop top", 0, 2000, 1);
		obs_properties_add_int_slider(propgroup, "cropBottom", "Crop bottom", 0, 2000, 1);
		obs_properties_add_int_slider(propgroup, "scale", "Scale", -1000, 1000, 1);
		//
		obs_properties_add_text(propgroup, "", "Cropping Tweaks:", OBS_TEXT_INFO);
		obs_properties_add_int_slider(propgroup, "panX", "Pan X", -1000, 1000, 1);
		obs_properties_add_int_slider(propgroup, "panY", "Pan Y", -1000, 1000, 1);
		obs_properties_add_int_slider(propgroup, "aspectRatio", "Aspect ratio", -1000, 1000, 1);
	}

	if (getIsPluginTypeSource()) {
		propgroup = obs_properties_create();
		obs_properties_add_group(props, "adjustments", "Targeting size+location", OBS_GROUP_NORMAL , propgroup);
		obs_properties_add_text(propgroup, "", "NOTE: Arrange border scene item immediately BEFORE (in front of) the source you wish to create a border around.", OBS_TEXT_INFO);
		obs_properties_add_bool(propgroup, "watchSignals", "Hook signals/events and auto-adjust size+pos (when previous scene item changes).");

		// manually adjust button
		obs_properties_add_button(propgroup, "adjustNow", "Adjust size and position now to match subsequent scene item", OnPropertyButtonClickAdjustNowCallback);
	}
	if (true) {
		// new background textures
		propgroup = obs_properties_create();
		obs_properties_add_group(props, "backTexture", "Background texture", OBS_GROUP_CHECKABLE , propgroup);

		std::string filterStr = std::string(TEXT_PATH_IMAGES) + std::string(IMAGE_FILTER_EXTENSIONS ";;") + std::string(TEXT_PATH_ALL_FILES) + std::string(" (*.*)");
		std::string startFolderPath = jrGetDirPathFromFilePath(opt_backTextureFilePath);
		obs_properties_add_path(propgroup, "backTextureFilePath", "Background texture file path", OBS_PATH_FILE, filterStr.c_str(), startFolderPath.c_str());

		obs_properties_add_int_slider(propgroup, "colorBlendLerpBack", "Background color blend %", 0, 100, 1);
	}

	return props;
}


bool OnPropertyButtonClickAdjustNowCallback(obs_properties_t* props, obs_property_t* property, void* data) {
	JrBorder *plugin = (JrBorder*) data;
	plugin->setNeedsShadowSourceReadjust(true, NULL);
	return true;
}


void JrBorder::gon_plugin_get_defaults(obs_data_t* settings) {
	obs_data_set_default_bool(settings, "border", true);
	obs_data_set_default_bool(settings, "cropzoom", true);
	obs_data_set_default_bool(settings, Setting_EnabledBorder, Setting_EnabledBorder_Def);
	obs_data_set_default_string(settings, Setting_borderStyle, Setting_borderStyle_Text);
	obs_data_set_default_string(settings, "shape", "rectangle");
	obs_data_set_default_bool(settings, "maskInside", true);
	obs_data_set_default_int(settings, Setting_borderWidth, Setting_borderWidth_Def);
	obs_data_set_default_int(settings, Setting_borderColor, Setting_borderColor_Def);
	//
	obs_data_set_default_bool(settings, Setting_EnabledCropZoom, Setting_EnabledCropZoom_Def);
	obs_data_set_default_int(settings, "cropLeft", 0);
	obs_data_set_default_int(settings, "cropRight", 0);
	obs_data_set_default_int(settings, "cropTop", 0);
	obs_data_set_default_int(settings, "cropBottom", 0);
	obs_data_set_default_int(settings, "scale", 0);
	obs_data_set_default_int(settings, "panX", 0);
	obs_data_set_default_int(settings, "panY", 0);
	obs_data_set_default_int(settings, "aspectRatio", 0);

	obs_data_set_default_int(settings, "borderoffset", 0);
	obs_data_set_default_bool(settings, "customColorSyntax", false);
	obs_data_set_default_string(settings, "borderformat", "1,3");
	obs_data_set_default_int(settings, Setting_borderColor, Setting_borderColor_Def);
	obs_data_set_default_int(settings, "colorBlendLerp", 50);
	obs_data_set_default_int(settings, "colorBlendLerpBack", 50);
	obs_data_set_default_int(settings, "color2", Setting_borderColor2_Def);
	obs_data_set_default_int(settings, "color3", Setting_borderColor3_Def);
	obs_data_set_default_int(settings, "color4", Setting_borderColor4_Def);
	//
	obs_data_set_default_string(settings, SettingCustomMaskType, "black vs white");
	obs_data_set_default_bool(settings, "aspectStretchCustomMask", false);
}


void JrBorder::gon_plugin_update(obs_data_t* settings) {
	// get options
	//opt_enable_border = obs_data_get_bool(settings, Setting_EnabledBorder);
	opt_enable_border = obs_data_get_bool(settings, "border");
	opt_borderStyle = jrPropertListChoiceFind(obs_data_get_string(settings, Setting_borderStyle), (const char**)Setting_borderStyle_Choices,0);
	opt_borderShape = jrPropertListChoiceFind(obs_data_get_string(settings, "shape"), (const char**)Setting_borderShape_Choices, 0);
	opt_maskInside = obs_data_get_bool(settings, "maskInside");
	opt_borderWidth = obs_data_get_int(settings, Setting_borderWidth);
	opt_borderColor = obs_data_get_int(settings, Setting_borderColor);
	opt_colorBlendLerp = obs_data_get_int(settings, "colorBlendLerp");
	opt_colorBlendLerpBack = obs_data_get_int(settings, "colorBlendLerpBack");
	//
	opt_customMaskType = (Setting_customMaskType_ChoicesEnum) jrPropertListChoiceFind(obs_data_get_string(settings, SettingCustomMaskType), (const char**)Setting_customMaskType_Choices,0);
	//
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

	opt_borderOffset = obs_data_get_int(settings, "borderoffset");
	opt_customColorSyntax = obs_data_get_bool(settings, "customColorSyntax");
	opt_borderFormat = std::string(obs_data_get_string(settings, "borderformat"));
	opt_borderColor2 = obs_data_get_int(settings, "color2");
	opt_borderColor3 = obs_data_get_int(settings, "color3");
	opt_borderColor4 = obs_data_get_int(settings, "color4");

	opt_flagWatchSignals = obs_data_get_bool(settings, "watchSignals");

	// new custom mask stuff
	std::string newCustomMaskFilePath = std::string(obs_data_get_string(settings, "maskFilePath"));
	if (opt_customMaskFilePath != newCustomMaskFilePath) {
		opt_customMaskFilePath = newCustomMaskFilePath;
		reloadCustomMaskFile();
	}
	opt_customMaskStretchAspect = obs_data_get_bool(settings, "aspectStretchCustomMask");

	// new back texture
	opt_backTextureEnabled = obs_data_get_bool(settings, "backTexture");
	std::string newBackTextureFilePath = std::string(obs_data_get_string(settings, "backTextureFilePath"));
	if (opt_backTextureFilePath != newBackTextureFilePath) {
		opt_backTextureFilePath = newBackTextureFilePath;
		reloadBackTextureFile();
	}

	// and now make changes based on options changing
	forceUpdatePluginSettingsOnOptionChange();

	// setup callbacks etc and trigger update
	sceneChangesSetupSceneCallbacks();
}
//---------------------------------------------------------------------------


























//---------------------------------------------------------------------------
JrBorder::JrBorder() : jrObsPluginSource() {
	bool success = init();
	if (!success) {
		mydebug("Failed to init.");
		return;
	}
	safetyCheckVal = DefJrBorderSafteyKludgeCheckVal;
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
	//mydebug("IN JrBorder Init.");

	sourceWidth = 0;
	renderSource = 0;

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
	texrenderBorder = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	texrenderBorderMask = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	texrenderBorderMaskTemp = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	texrenderTemp = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
	texrenderCustomMask = gs_texrender_create(GS_RGBA, GS_ZS_NONE);

	return success;
}



void JrBorder::deInitialize() {

	//mydebug("IN JrBorder Deinit.");

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
		if (effectCustomMask) {
			gs_effect_destroy(effectCustomMask);
			effectCustomMask = NULL;
		}
		//
		if (texrenderSource) {
			gs_texrender_destroy(texrenderSource);
			texrenderSource = NULL;
		}
		if (texrenderBorder) {
			gs_texrender_destroy(texrenderBorder);
			texrenderBorder = NULL;
		}
		if (texrenderBorderMask) {
			gs_texrender_destroy(texrenderBorderMask);
			texrenderBorderMask = NULL;
		}
		if (texrenderBorderMaskTemp) {
			gs_texrender_destroy(texrenderBorderMaskTemp);
			texrenderBorderMaskTemp = NULL;
		}
		if (texrenderTemp) {
			gs_texrender_destroy(texrenderTemp);
			texrenderTemp = NULL;
		}
		if (texrenderCustomMask) {
			gs_texrender_destroy(texrenderCustomMask);
			texrenderCustomMask = NULL;
		}

		obs_leave_graphics();
	}

	// free stuff
	freeCustomMaskFile();
	freeBackTextureFile();

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

	effectPath = obs_module_file("jrborder_mask.effect");
	effectCustomMask = gs_effect_create_from_file(effectPath, NULL);
	bfree(effectPath);
	if (!effectCustomMask) {
		mydebug("ERROR loading effectCustomMask effect.");
		return false;
	}

	// crop
	param_crop_mulVal = gs_effect_get_param_by_name(effectCropScale, "mulVal");
	param_crop_addVal = gs_effect_get_param_by_name(effectCropScale, "addVal");

	// border

	param_border_color = gs_effect_get_param_by_name(effectBorder, "border_color");
	param_border_mulVal = gs_effect_get_param_by_name(effectBorder, "mulVal");
	param_border_addVal = gs_effect_get_param_by_name(effectBorder, "addVal");

	param_borderPatternCount = gs_effect_get_param_by_name(effectBorder, "borderPatternCount");
	param_color1 = gs_effect_get_param_by_name(effectBorder, "color1");
	param_color2 = gs_effect_get_param_by_name(effectBorder, "color2");
	param_color3 = gs_effect_get_param_by_name(effectBorder, "color3");
	param_color4 = gs_effect_get_param_by_name(effectBorder, "color4");
	param_borders[0] = gs_effect_get_param_by_name(effectBorder, "b1");
	param_borders[1] = gs_effect_get_param_by_name(effectBorder, "b2");
	param_borders[2] = gs_effect_get_param_by_name(effectBorder, "b3");
	param_borders[3] = gs_effect_get_param_by_name(effectBorder, "b4");
	param_innerBorderEdge = gs_effect_get_param_by_name(effectBorder, "borderInnerEdge");

	param_colorMarkExclude = gs_effect_get_param_by_name(effectBorder, "ColorMaskExclude");
	param_colorMarkInclude = gs_effect_get_param_by_name(effectBorder, "ColorMaskInclude");
	//
	param_circularMaskDist = gs_effect_get_param_by_name(effectBorder, "circularMaskDist");
	param_innerLimits = gs_effect_get_param_by_name(effectBorder, "innerLimits");
	param_colorBlendLerpBack = gs_effect_get_param_by_name(effectBorder, "colorBlendLerpBack");

	return true;
}
//---------------------------------------------------------------------------

































//---------------------------------------------------------------------------
bool JrBorder::gon_plugin_render(gs_effect_t* obsoleteFilterEffect) {
	if (getIsPluginTypeFilter()) {
		// filter mode
		return doRenderPluginFilter();
	}
	else if (getIsPluginTypeSource()) {
		// source mode
		return doRenderPluginSource();
	}
	return false;
}
//---------------------------------------------------------------------------












































































//---------------------------------------------------------------------------
void JrBorder::registerCallbacksAndHotkeys() {
	jrObsPlugin::registerCallbacksAndHotkeys();
	obs_frontend_add_event_callback(ObsFrontendEvent, this);
}

void JrBorder::unregisterCallbacksAndHotkeys() {
	obs_frontend_remove_event_callback(ObsFrontendEvent, this);
}

// statics just reroute to a cast object member function call

void JrBorder::ObsFrontendEvent(enum obs_frontend_event event, void *ptr)
{
	JrBorder *pluginp = reinterpret_cast<JrBorder *>(ptr);
	pluginp->handleObsFrontendEvent(event);
}


void JrBorder::handleObsFrontendEvent(enum obs_frontend_event event) {
	switch ((int)event) {
		case OBS_FRONTEND_EVENT_SCENE_CHANGED:
			sceneChangesSetupSceneCallbacks();
			break;
	}

	// let parent handle some cases
	jrObsPluginSource::handleObsFrontendEvent(event);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// see signal_handler_connect at https://docs.obsproject.com/reference-libobs-callback
void JrBorder::sceneChangesSetupSceneCallbacks() {
	if (!getIsPluginTypeSource()) {
		// this is only used for sources
		return;
	}
	if (getIsSingletonRep()) {
		// singleton doesnt worry about this
		return;
	}

	// on scene change we setup callback to hear about transforms on this scene

	// release any existing
	releaseSceneCallbacks();

	if (!opt_flagWatchSignals) {
		// do not deal with signals at all
		return;
	}

	// borrow pointer to scene source
	obs_source_t* sceneSource = obs_frontend_get_current_scene();
	if (sceneSource == NULL) {
		return;
	}

	// get signal handler for scene
	sourceSceneSignalHandlerp = obs_source_get_signal_handler(sceneSource);

	// now we want to attach
	signal_handler_connect(sourceSceneSignalHandlerp, "item_transform", onSceneItemTransformChangeHandle, this);

	// free
	obs_source_release(sceneSource);

	// also trigger readjust on changing scenes
	setNeedsShadowSourceReadjust(true, NULL);
}


void JrBorder::releaseSceneCallbacks() {
	// let go of any previously attached handler
	if (sourceSceneSignalHandlerp!=NULL) {
		signal_handler_disconnect(sourceSceneSignalHandlerp, "item_transform", onSceneItemTransformChangeHandle, this);
		sourceSceneSignalHandlerp = NULL;
	}
}


void JrBorder::sceneCallbackTriggers(obs_sceneitem_t* itemp) {
	// adjust size and position since something has changed
	//
	if (true) {
		// surgical delayed scan
		setNeedsShadowSourceReadjust(true, itemp);
	} else {
		// surgical immediate scan
		rescanCurrentSceneAndAdjustSizePositions(itemp);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void onSceneItemTransformChangeHandle(void* vp, calldata_t *cdatap) {
	JrBorder *pluginp = reinterpret_cast<JrBorder *>(vp);
	if (pluginp == NULL) {
		mydebug("NUll void pointer cast to JrBorder* in onSceneItemTransformChangeHandle");
	}

	// we can have a pointer to the changed item
	obs_sceneitem_t* itemp = reinterpret_cast<obs_sceneitem_t*> (calldata_ptr(cdatap, "item"));
	pluginp->sceneCallbackTriggers(itemp);
}
//---------------------------------------------------------------------------












//---------------------------------------------------------------------------
void JrBorder::rescanCurrentSceneAndAdjustSizePositions(obs_sceneitem_t* itemp) {

	//mydebug("In rescan in plugin this = %p and itemp = %p.", this, itemp);

	// helper data structure
	struct TIterateStruct {
		obs_sceneitem_t* borderSceneItemp;
		obs_sceneitem_t* previousSceneItemp = NULL;
		bool triggerOnNext = false;
		obs_sceneitem_t* changeditemp = NULL;
	};
	TIterateStruct iterateStruct;

	// callback used on each scene item
	auto cbPrior = [](obs_scene_t*, obs_sceneitem_t* sceneItem, void* param) {
		TIterateStruct* datap = (TIterateStruct*)param;
		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
		auto source_id = obs_source_get_unversioned_id(itemSource);
		//
		// is this source a special border source of ours, if so, remember it!
		// are we looking for the next source AFTER a border source
		if (strcmp(source_id, PLUGIN_NAME_Source)==0) {
			// one of us
			datap->triggerOnNext = true;
			datap->borderSceneItemp = sceneItem;
		}
		else if (datap->triggerOnNext) {
			// clear it
			datap->triggerOnNext = false;
			if (datap->changeditemp == NULL || datap->changeditemp == sceneItem) {
				// tell our border source to resize based on this one! with lots of reference protecting:
				OBSSource borderSourceBig = obs_sceneitem_get_source(datap->borderSceneItemp);
				// how do we get a pointer to our plugin* which should be stored with the source
				JrBorder* borderTargetp = (JrBorder*)jrobsGetVoidPointerToSourceContextDataPluginp(borderSourceBig);
				if (!borderTargetp->passesSafetySanityCheck()) {
					mydebug("ERROR: JrBorder prev source kludge pointer failed safety check; aborting.");
					return false;
				}
				borderTargetp->updateSizePosFromSceneItem(datap->borderSceneItemp, sceneItem);
			}
		}
		return true;
	};


	// callback used on each scene item
	auto cbNext = [](obs_scene_t*, obs_sceneitem_t* sceneItem, void* param) {
		TIterateStruct* datap = (TIterateStruct*)param;
		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
		auto source_id = obs_source_get_unversioned_id(itemSource);
		//
		// is this source a special border source of ours, if so, remember it!
		// are we looking for the next source AFTER a border source
		if (strcmp(source_id, PLUGIN_NAME_Source)==0 && datap->previousSceneItemp!=NULL) {
			// one of us
			datap->borderSceneItemp = sceneItem;
			// the item we shadow (then clear it so it doesnt get tackled again)
			obs_sceneitem_t* shadowedScenep = datap->previousSceneItemp;
			datap->previousSceneItemp = NULL;
			//
			if (datap->changeditemp == NULL || datap->changeditemp == shadowedScenep) {
				// tell our border source to resize based on this one! with lots of reference protecting:
				OBSSource borderSourceBig = obs_sceneitem_get_source(datap->borderSceneItemp);
				// how do we get a pointer to our plugin* which should be stored with the source
				JrBorder* borderTargetp = (JrBorder*)jrobsGetVoidPointerToSourceContextDataPluginp(borderSourceBig);
				if (!borderTargetp->passesSafetySanityCheck()) {
					mydebug("ERROR: JrBorder next source kludge pointer failed safety check; aborting.");
					return false;
				}
				borderTargetp->updateSizePosFromSceneItem(datap->borderSceneItemp, shadowedScenep);
			}

		}
		else {
			// remember this one for next time
			datap->previousSceneItemp = sceneItem;
		}
		return true;
	};

	obs_source_t* sceneSource = obs_frontend_get_current_scene();
	if (sceneSource == NULL) {
		return;
	}
	obs_scene_t* scenep = obs_scene_from_source(sceneSource);

	// set any iteratestuct data values
	iterateStruct.changeditemp = itemp;

	doRunObsCallbackOnScene(scenep, cbNext, &iterateStruct, true);
	obs_source_release(sceneSource);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrBorder::setNeedsShadowSourceReadjust(bool val, obs_sceneitem_t* targetitemp) {
	needsShadowSourceReadjust = val;
	// ok now we try to be clever to only watch for changes to a target shadowed item that changed
	// BUT if multiple different items change within the same cycle, then we switch to global watch
	if (val == false) {
		// clear
		flagFallbackGlobalScan = false;
		needsReadjustTargetItemp = NULL;
		return;
	}
	if (needsReadjustTargetItemp == NULL && !flagFallbackGlobalScan) {
		// target this one specifically
		needsReadjustTargetItemp = targetitemp;
	}
	else {
		// multiple targets, so clear target and set flag so we stay cleared
		needsReadjustTargetItemp = NULL;
		flagFallbackGlobalScan = true;
	}
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrBorder::updateSizePosFromSceneItem(obs_sceneitem_t* borderSceneItemp, obs_sceneitem_t* shadowedSceneItemp) {
	float bwidth, bheight;

	OBSSource borderSourcep = obs_sceneitem_get_source(borderSceneItemp);
	OBSSource shadowSourcep = obs_sceneitem_get_source(shadowedSceneItemp);

	struct obs_transform_info inShadowTransform { 0 };
	struct obs_sceneitem_crop inShadowCrop { 0 };
	struct obs_transform_info outBorderTransform { 0 };
	struct obs_sceneitem_crop outBorderCrop { 0 };
	struct vec2 inShadowBounds { 0 };
	struct vec2 outBorderBounds { 0 };

	struct obs_transform_info outBorderTransformOrig;
	struct obs_sceneitem_crop outBorderCropOrig;
	struct vec2 outBorderBoundsOrig;

	// get shadowed object transform
	obs_sceneitem_get_info(shadowedSceneItemp, &inShadowTransform);
	obs_sceneitem_get_crop(shadowedSceneItemp, &inShadowCrop);
	obs_sceneitem_get_bounds(shadowedSceneItemp, &inShadowBounds);

	// get current
	obs_sceneitem_get_info(borderSceneItemp, &outBorderTransformOrig);
	obs_sceneitem_get_crop(borderSceneItemp, &outBorderCropOrig);
	obs_sceneitem_get_bounds(borderSceneItemp, &outBorderBoundsOrig);

	// start by copying
	outBorderTransform = inShadowTransform;

	// details
	float scalex = inShadowTransform.scale.x;
	float scaley = inShadowTransform.scale.y;

	// final size?
	//width = inShadowBounds.x+opt_borderWidth*2;
	//height = inShadowBounds.y+opt_borderWidth*2;

	float swidth = obs_source_get_width(shadowSourcep) - (inShadowCrop.left + inShadowCrop.right);
	float sheight = obs_source_get_height(shadowSourcep) - (inShadowCrop.top + inShadowCrop.bottom);

	float swidthScaled, sheightScaled;

	if (true) {
		// set our scale to 1 and rescale everything
		// we need to do this because other calculations here and elsewhere rely on borderwidth being per pixel so it doesnt change with scale of source
		// the alternative would be to keep a scale factor for ourselves and modify such border calculations based on thaat
		ourSourceScaleX = 1.0;
		ourSourceScaleY = 1.0;
		swidthScaled = swidth * scalex;
		sheightScaled = sheight * scaley;
	}
	else {
		swidthScaled = swidth;
		sheightScaled = sheight;
		ourSourceScaleX = scalex;
		ourSourceScaleY = scaley;
	}

	// save
	outBorderTransform.scale.x = ourSourceScaleX;
	outBorderTransform.scale.y = ourSourceScaleY;

	// attempt to handle source border edges
	//swidthScaled = ceil(swidthScaled);
	//sheightScaled = ceil(sheightScaled);

	shadowedWidth = swidthScaled;
	shadowedHeight = sheightScaled;

	float borderWidthScaledX = (float)opt_borderWidth * ourSourceScaleX;
	float borderWidthScaledY = (float)opt_borderWidth * ourSourceScaleY;
	float offsetPixels = calcOffsetPixels();
	float offsetPixelsScaledX = (float)offsetPixels * ourSourceScaleX;
	float offsetPixelsScaledY = (float)offsetPixels * ourSourceScaleY;

	// our border size
	bwidth = swidthScaled + borderWidthScaledX*2;
	bheight = sheightScaled + borderWidthScaledY*2;


	// resets
	outBorderCrop.left = 0;
	outBorderCrop.right = 0;
	outBorderCrop.top = 0;
	outBorderCrop.bottom = 0;

	float shadowX = inShadowTransform.pos.x;
	float shadowY = inShadowTransform.pos.y;


	// offset
	bwidth -= offsetPixelsScaledX * 2.0;
	bheight -= offsetPixelsScaledY * 2.0;

	//
	// convert to center
	if (true) {
		// this works BUT rotation breaks
		if ((inShadowTransform.alignment & OBS_ALIGN_LEFT) != 0) {
			shadowX += swidthScaled / 2.0f;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_RIGHT) != 0) {
			shadowX -= swidthScaled / 2.0f;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_TOP) != 0) {
			shadowY += sheightScaled / 2.0f;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_BOTTOM) != 0) {
			shadowY -= sheightScaled / 2.0f;
		}
		outBorderTransform.alignment = OBS_ALIGN_CENTER;
	}
	else {
		// match shadow alignment?
		if ((inShadowTransform.alignment & OBS_ALIGN_LEFT) != 0) {
			shadowX -= borderWidthScaledX-offsetPixelsScaledX;
		} else if ((inShadowTransform.alignment & OBS_ALIGN_RIGHT) != 0) {
			shadowX += borderWidthScaledX-offsetPixelsScaledX;
		} else {

		}
		if ((inShadowTransform.alignment & OBS_ALIGN_TOP) != 0) {
			shadowY -= borderWidthScaledY-offsetPixelsScaledY;
		} else if ((inShadowTransform.alignment & OBS_ALIGN_BOTTOM) != 0) {
			shadowY += borderWidthScaledY-offsetPixelsScaledY;
		} else {

		}
		outBorderTransform.alignment = inShadowTransform.alignment;
	}


	// position
	outBorderTransform.pos.x = shadowX;
	outBorderTransform.pos.y = shadowY;

	// other resets
	outBorderTransform.bounds_type = OBS_BOUNDS_NONE;

	// set changes (only set changed values to not trigger a repeat)
	// im not sure memcmp is appropriate for transform struct

	/*
	if (memcmp(&outBorderTransformOrig, &outBorderTransform, sizeof(outBorderTransform) != 0) || outBorderTransform.pos.x != outBorderTransformOrig.pos.x || outBorderTransform.pos.y != outBorderTransformOrig.pos.y || outBorderTransform.scale.x != outBorderTransformOrig.scale.x || outBorderTransform.scale.y != outBorderTransformOrig.scale.y) {
		obs_sceneitem_set_info(borderSceneItemp, &outBorderTransform);
	}
	if (memcmp(&outBorderCropOrig, &outBorderCrop, sizeof(outBorderCrop) != 0)) {
		obs_sceneitem_set_crop(borderSceneItemp, &outBorderCrop);
	}
	if (memcmp(&outBorderBoundsOrig, &outBorderBounds, sizeof(outBorderBounds) != 0)) {
		obs_sceneitem_set_bounds(borderSceneItemp, &outBorderBounds);
	}
	*/

	if (outBorderTransform.pos.x != outBorderTransformOrig.pos.x || outBorderTransform.pos.y != outBorderTransformOrig.pos.y || outBorderTransform.scale.x != outBorderTransformOrig.scale.x || outBorderTransform.scale.y != outBorderTransformOrig.scale.y) {
		obs_sceneitem_set_info(borderSceneItemp, &outBorderTransform);
	}
	if (outBorderCropOrig.left != outBorderCrop.left || outBorderCropOrig.top != outBorderCrop.top || outBorderCropOrig.right != outBorderCrop.right || outBorderCropOrig.bottom != outBorderCrop.bottom) {
		obs_sceneitem_set_crop(borderSceneItemp, &outBorderCrop);
	}
	
	int bwidthi = round(bwidth);
	int bheighti = round(bheight);

	if (bwidthi != cropScaleStageFinalWidth || bheighti != cropScaleStageFinalHeight) {
		// change in size
		cropScaleStageFinalWidth = bwidthi;
		cropScaleStageFinalHeight = bheighti;
		// no need to set these, they will update on next render
		//width = cropScaleStageFinalWidth;
		//height = cropScaleStageFinalHeight;
		// and because we have changed, we need to make sure border texture is recreated
		markBorderTextureCacheInvalid();
	}

}
//---------------------------------------------------------------------------




















//---------------------------------------------------------------------------
void JrBorder::doPrecalcScaleZoomBorderStuff(int swidth, int sheight) {
	// even in source mode run this
	doPrecalcCropScale(swidth, sheight);

	// border effects
	doSetEffectParamsBorder(cropScaleStageFinalWidth, cropScaleStageFinalHeight);
}




void JrBorder::doSetEffectParamsBorder(int swidth, int sheight) {
	// set uniform parameters


	// circular stuff
	float borderSizeMultiplier = 1.0f;
	float circularMaskDist = 1.0f;
	//
	if (opt_borderShape == borderShapeEnumCircle) {
		if (swidth < sheight) {
			borderSizeMultiplier = 1.0;
		}
		else {
			borderSizeMultiplier = 1.0;
		}
	}


	// set colors
	struct vec4 borderColorVec;
	jrazUint32ToRgbaVec(opt_borderColor, borderColorVec);
	gs_effect_set_vec4(param_border_color, &borderColorVec);



	// fill other colors
	gs_effect_set_vec4(param_color1, &borderColorVec);
	jrazUint32ToRgbaVec(opt_borderColor2, borderColorVec);
	gs_effect_set_vec4(param_color2, &borderColorVec);
	jrazUint32ToRgbaVec(opt_borderColor3, borderColorVec);
	gs_effect_set_vec4(param_color3, &borderColorVec);
	jrazUint32ToRgbaVec(opt_borderColor4, borderColorVec);
	gs_effect_set_vec4(param_color4, &borderColorVec);

	// fill min max with the border pattern
	std::vector<float> borderWidthsRaw;
	int counts = 0;

	if (opt_customColorSyntax) {
		bool bretv = jrParseCommaStringVector(opt_borderFormat, borderWidthsRaw);

		// precalcs
		counts = (int)(borderWidthsRaw.size());
	}
	else {
		// just fall down and use one color for border
	}

	if (counts == 0) {
		++counts;
		borderWidthsRaw.push_back(1.0f);
	}
	if (counts > DefJrBorderMaxBorders) {
		counts = DefJrBorderMaxBorders;
	}
	gs_effect_set_int(param_borderPatternCount, counts);

	double sum = std::accumulate(borderWidthsRaw.begin(), borderWidthsRaw.end(), 0);
	if (sum <= 0) {
		sum = 1.0f;
	}
	double widthMultiplier = (double)opt_borderWidth / (double)sum;
	//
	double perPixelX = (double)1.0f / swidth;
	double perPixelY = (double)1.0f / sheight;

	// ok now we can iterate through the border widths and assin min maxs


	// set
	struct vec4 borderpos { 0 };
	double offset{ 0.0f };
	double minx, miny, maxx, maxy;
	for (int i = 0; i < counts; ++i) {
		double pwidth = borderWidthsRaw[i] * widthMultiplier;
		if (pwidth >0.0 && pwidth < 1.0) {
			pwidth = 1.0;
		}
		offset += pwidth;
		minx = offset * perPixelX * borderSizeMultiplier;
		miny = offset * perPixelY * borderSizeMultiplier;
		maxx = 1.0f - minx;
		maxy = 1.0f - miny;
		borderpos.ptr[0] = minx;
		borderpos.ptr[1] = miny;
		borderpos.ptr[2] = maxx;
		borderpos.ptr[3] = maxy;
		gs_effect_set_vec4(param_borders[i], &borderpos);
		//mydebug("border %d is [%f,%f] - [%f,%f].", i, minx, miny, maxx, maxy);
	}
	// set all effect uniforms
	for (int i = counts; i < DefJrBorderMaxBorders; ++i) {
		borderpos.ptr[0] = -1;
		borderpos.ptr[1] = -1;
		borderpos.ptr[2] = 2;
		borderpos.ptr[3] = 2;
		gs_effect_set_vec4(param_borders[i], &borderpos);
	}

	// set last border pos
	borderpos.ptr[0] = minx;
	borderpos.ptr[1] = miny;
	borderpos.ptr[2] = maxx;
	borderpos.ptr[3] = maxy;
	gs_effect_set_vec4(param_innerBorderEdge, &borderpos);

	if (opt_maskInside) {
		// mask inner goes to boundary of most inner cutoff
		circularMaskDist = borderpos.ptr[3];
	} else {
		// mask at EDGE
		circularMaskDist = 1.0f;
	}
	// 
	gs_effect_set_float(param_circularMaskDist, circularMaskDist);

	// constants
	jrazUint32ToRgbaVec(DefMaskColorExclude, borderColorVec);
	gs_effect_set_vec4(param_colorMarkExclude, &borderColorVec);
	jrazUint32ToRgbaVec(DefMaskColorInclude, borderColorVec);
	gs_effect_set_vec4(param_colorMarkInclude, &borderColorVec);

	// inner limits -- no masking outside of this area, which is the original image
	vec4 innerLimits;
	setInnerLimitsOriginalImage(innerLimits);
	gs_effect_set_vec4(param_innerLimits, &innerLimits);

	float colorBlendLerpBack = (float)opt_colorBlendLerpBack / 100.0f;
	gs_effect_set_float(param_colorBlendLerpBack, colorBlendLerpBack);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool JrBorder::updateBorderComputationsCacheAndBorderTexture(int swidth, int sheight) {
	//mydebug("ATTN: in updateBorderComputationsCacheAndBorderTexture 1 %dx%d", swidth, sheight);
	if (swidth <= 0 || sheight <= 0) {
		// sanity check
		flagUsingBorder = false;
		flagUsingBorderMask = false;
		return false;
	}

	gs_texture_t* backTexturep = NULL;
	std::string drawTechniqueSuffix;

	if (opt_backTextureEnabled) {
		backTexturep = backTexture;
		drawTechniqueSuffix = "BackTex";
	}

	bool rebuilt = false;
	if (!isBorderTextureCacheValid(swidth, sheight)) {
		// reset flags until we create them
		flagUsingBorder = false;
		flagUsingBorderMask = false;

		// effect parameters (both border and cropscale if filter)
		doPrecalcScaleZoomBorderStuff(swidth, sheight);

		//mydebug("ATTN: in updateBorderComputationsCacheAndBorderTexture 3 (%dx%d)", cropScaleStageFinalWidth, cropScaleStageFinalHeight);

		if (cropScaleStageFinalWidth <= 0 || cropScaleStageFinalHeight <= 0) {
			flagUsingBorder = false;
			flagUsingBorderMask = false;
			return false;
		}


		// re-render border
		// ATTN: new, we always render into border texture holding area so we can cache
		//jrRenderEffectIntoTextureT(texrenderBorder, effectBorder, backTexturep, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendClearOverwite, "Draw");
		if (opt_borderShape==borderShapeEnumCircle) {
			// build mask
			jrRenderEffectIntoTextureT(texrenderBorderMask, effectBorder, backTexturep, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, "DrawCircleMask");
			flagUsingBorderMask = true;
			// why do we have to RESET params???
			// i think its because we use this SAME filter on other sources, and obs must SHARE it
			doSetEffectParamsBorder(cropScaleStageFinalWidth, cropScaleStageFinalHeight);
			jrRenderEffectIntoTextureT(texrenderBorder, effectBorder, backTexturep, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, ("DrawCircle"+drawTechniqueSuffix).c_str());
			flagUsingBorder = true;
		} else if (opt_borderShape==borderShapeEnumRectangle) {
			// rectangle does not need a mask
			// oh yes it does if we are trying to crop inside
			if (opt_maskInside) {
				// build mask
				jrRenderEffectIntoTextureT(texrenderBorderMask, effectBorder, backTexturep, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, "DrawRectangleMask");
				flagUsingBorderMask = true;
				// why do we have to RESET params???
				// i think its because we use this SAME filter on other sources, and obs must SHARE it
				doSetEffectParamsBorder(cropScaleStageFinalWidth, cropScaleStageFinalHeight);
				jrRenderEffectIntoTextureT(texrenderBorder, effectBorder, backTexturep, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, ("DrawRectangle"+drawTechniqueSuffix).c_str());
				flagUsingBorder = true;
			}
			else {
				// no mask needed
				jrRenderEffectIntoTextureT(texrenderBorder, effectBorder, backTexturep, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, ("DrawRectangle"+drawTechniqueSuffix).c_str());
				flagUsingBorderMask = false;
				flagUsingBorder = true;
			}
		} else if (opt_borderShape==borderShapeEnumCustom) {
			// build mask
			buildCustomMaskAndBorder();
		} else {
			// error unknown
		}

		// remember cached values so we dont have to redraw border again if this doesn't change
		cachedBorderWidth = swidth;
		cachedBorderHeight = sheight;
		rebuilt = true;
		//mydebug("ATTN: in updateBorderComputationsCacheAndBorderTexture 3b (%dx%d)", cropScaleStageFinalWidth, cropScaleStageFinalHeight);
	}

	// new set this from computations so we dont have to always run
	width = cropScaleStageFinalWidth;
	height = cropScaleStageFinalHeight;

	//mydebug("ATTN: in updateBorderComputationsCacheAndBorderTexture 4 (%dx%d)", cropScaleStageFinalWidth, cropScaleStageFinalHeight);

	return rebuilt;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void JrBorder::doRenderEffectCropScale(gs_texrender_t* texDest, gs_texrender_t* texSource, int swidth, int sheight) {
	// note that this used PRECOMPUTED values so that we only have to recompute on changed size or options
	doSetEffectParamsCropScale();
	if (DefJrAvoidAlphaMaskDrawingUnnescesarily) {
		jrRenderEffectIntoTextureAtSizeLoc(texDest, effectCropScale, texSource, NULL, swidth, sheight, cropScaleStageOffx, cropScaleStageOffy, cropScaleStageCoreWidth, cropScaleStageCoreHeight, jrBlendPureCopy, "Draw", width, height);
	}
	else {
		jrRenderEffectIntoTextureAtSizeLoc(texDest, effectCropScale, texSource, NULL, swidth, sheight, cropScaleStageOffx, cropScaleStageOffy, cropScaleStageCoreWidth, cropScaleStageCoreHeight, jrBlendSrcAlphaMerge, "Draw", width, height);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void JrBorder::doSetEffectParamsCropScale() {
	gs_effect_set_vec2(param_crop_addVal, &cropScaleStage_offVal);
	gs_effect_set_vec2(param_crop_mulVal, &cropScaleStage_mulVal);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrBorder::doPrecalcCropScale(int swidth, int sheight) {
	// compute values needed for cropscale and final stuff
	// for crop scale we first can apply a crop to the source image in pixels
	// then we scale the image up or down
	int borderWidthAdjusted = calcAdjustedOffset();

	if (!getIsPluginTypeFilter()) {
		// SOURCE TYPE -- ie standalone source
		// source types get their size differnetly for scaling reasons
		cropScaleStageOffx = 0;
		cropScaleStageOffy = 0;
		cropScaleStageCoreWidth = swidth;
		cropScaleStageCoreHeight = sheight;
		// importantly, cropScaleStageFinalWidth is not set here
		return;
	}

	// OK SO WE ARE A FILTER ATTACHED TO A SOURCE

	if (!isEnabledCropOrZoom()) {
		// cropzoom is disabled, so its at original size
		cropScaleStageOffx = borderWidthAdjusted;
		cropScaleStageOffy = borderWidthAdjusted;
		cropScaleStageCoreWidth = swidth;
		cropScaleStageCoreHeight = sheight;
		cropScaleStageFinalWidth = cropScaleStageCoreWidth + (borderWidthAdjusted * 2);
		cropScaleStageFinalHeight = cropScaleStageCoreHeight + (borderWidthAdjusted * 2);
		return;
	}

	// getarget dimensions for crop, before scale
	int renderedWidth = swidth - (opt_cropLeft + opt_cropRight);
	int renderedHeight = sheight - (opt_cropTop + opt_cropBottom);

	// crop effects
	cropScaleStage_offVal.x = (float)opt_cropLeft/(float)swidth;
	cropScaleStage_offVal.y = (float)opt_cropTop / (float)sheight;
	cropScaleStage_mulVal.x = (float) renderedWidth / (float)swidth;
	cropScaleStage_mulVal.y = (float) renderedHeight / (float)sheight;

	// scaling mod on top
	if (opt_enable_cropzoom && (opt_scale != 0 || opt_aspectRatio!=0)) {
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

	if (isEnabledBorder() && opt_borderStyle == borderStyleEnumConstrain) {
		// this is a kludge to have US to the resize to convert a shrink mode border to an outer border to avoid scaling twice
		float borderWidthAdjusted = calcAdjustedOffset();
		float scaleMultX = (float)(renderedWidth - borderWidthAdjusted * 2) / (float)renderedWidth;
		float scaleMultY = (float)(renderedHeight - borderWidthAdjusted * 2) / (float)renderedHeight;
		renderedWidth *= scaleMultX;
		renderedHeight *= scaleMultY;
	}

	// offset render of source to make room for border
	cropScaleStageOffx = borderWidthAdjusted;
	cropScaleStageOffy = borderWidthAdjusted;
	// adjust final output size for next step of filter to consider for border
	cropScaleStageCoreWidth = renderedWidth;
	cropScaleStageCoreHeight = renderedHeight;
	cropScaleStageFinalWidth = cropScaleStageCoreWidth + (borderWidthAdjusted * 2);
	cropScaleStageFinalHeight = cropScaleStageCoreHeight + (borderWidthAdjusted * 2);
}
//---------------------------------------------------------------------------












































//---------------------------------------------------------------------------
bool JrBorder::doRenderPluginSource() {
	// calc settings for scene
	updateSourceProperties();

	//startLinearRgbSpaceMode();
	//mydebug("doRenderPluginSource1 %dx%d (tw = %dx%d)", sourceWidth, sourceHeight, width, height);

	if (needsShadowSourceReadjust) {
		// get any taget item to watch for changes (can be null meaning assume its changed)
		obs_sceneitem_t* targetItemp = needsReadjustTargetItemp;
		// clear
		setNeedsShadowSourceReadjust(false, NULL);
		// scan
		rescanCurrentSceneAndAdjustSizePositions(targetItemp);
		//mydebug("doRenderPluginSource2");
	}

	if (!effectBorder || width<=0 || height<=0) {
		// dont render normal source, better to show there is error with blankness
		//mydebug("doRenderPluginSource3");
		return false;
	}

	// update cache computations, etc.
	bool wasRebuilt = updateBorderComputationsCacheAndBorderTexture(sourceWidth, sourceHeight);

	// new set this from computations so we dont have to always run
	//width = cropScaleStageFinalWidth;
	//height = cropScaleStageFinalHeight;

	//if (isEnabledAndWorkingBorder()) {
	if (isEnabledAndWorkingBorder()) {
		// and now from border texture (cached)
		//mydebug("doRenderPluginSource4");
		if (isEnabledAndWorkingBorderMask()) {
			//mydebug("doRenderPluginSource5");
			// mask out first
			// see https://www.andersriggelsen.dk/glblendfunc.php
			if (true) {
				jrRenderTextureIntoTexture(NULL, texrenderBorderMask, width, height, jrBlendSrcAlphaMask2);
			}
			else {
				jrRenderTextureIntoTexture(NULL, texrenderBorderMask, width, height, jrBlendSrcAlphaMask);
			}
		}
		// now render border
		//jrRenderTextureIntoTexture(NULL, texrenderBorder, width, height, jrBlendSrcAlphaMerge);
		jrRenderTextureIntoTexture(NULL, texrenderBorder, width, height, jrBlendSrcObsSep);
	}

	//endLinearRgbSpaceMode();
	//mydebug("doRenderPluginSource6");
	return true;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
bool JrBorder::doRenderPluginFilter() {
	// this is called for FILTER type plugin
	// calc settings for scene
	updateSourceProperties();

	if (!effectBorder || !effectCropScale || !renderSource || sourceWidth == 0 || sourceHeight == 0) {
		// dont render normal source, better to show there is error with blankness
		return false;
	}

	if (!isEnabledBorder() && !isEnabledCropOrZoom()) {
		// nothing to do
		if (true) {
			// if we are a filter then there is nothing to do
			// this is how to say don't render filter
			obs_source_skip_video_filter(context);
		}
		else {
			// test method to render source directly; we dont use this
			jrRenderSourceOut(renderSource, sourceWidth, sourceHeight, sourceWidth, sourceHeight, jrBlendSrcAlphaMerge);
		}
		width = sourceWidth;
		height = sourceHeight;
		return true;
	}

	// NULL to render direct to screen, but sometimes we need to use temp buffer
	gs_texrender_t* outTexrender = NULL;
	bool flag_renderToTempForCompositing = true;
	if (flag_renderToTempForCompositing) {
		outTexrender = texrenderTemp;
	}

	//startLinearRgbSpaceMode();

	// update cache computations, etc. (this will render the border into our border cache texture)
	bool wasRebuilt = updateBorderComputationsCacheAndBorderTexture(sourceWidth, sourceHeight);

	// if rendering to our temp buffer we need to clear it each time
	if (outTexrender != NULL) {
		// must be cleared each time
		if (wasRebuilt || !DefJrAvoidAlphaMaskDrawingUnnescesarily) {
			// ATTN: not sure this is really needed
			jrDrawTextureClear(outTexrender, width, height);
		}
	}

	// ATTN: TODO -- if this filter does not use cropzoom, can we simplify this to a single step?
	// it's not so simple because doRenderEffectCropScale can effect offset of drawing based on border..
	if (isEnabledCropOrZoom()) {
		// draw source into holding texture first with default effect
		jrRenderSourceIntoTexture(renderSource, texrenderSource, sourceWidth, sourceHeight, jrBlendClearOverwite);
		// ok render the source to output texture (or display) at desired crop/size/position
		doRenderEffectCropScale(outTexrender, texrenderSource, sourceWidth, sourceHeight);
	}
	else {
		// single render of source bypassing cropscale effect? the only trick here is that we may have to OFFSET it but we wont be scaling/resizing it
		if (DefJrAvoidAlphaMaskDrawingUnnescesarily) {
			jrRenderSourceIntoTextureAtSizeLoc(renderSource, outTexrender, sourceWidth, sourceHeight, cropScaleStageOffx, cropScaleStageOffy, sourceWidth, sourceHeight, jrBlendPureCopy, false, width, height);
			}
		else {
			jrRenderSourceIntoTextureAtSizeLoc(renderSource, outTexrender, sourceWidth, sourceHeight, cropScaleStageOffx, cropScaleStageOffy, sourceWidth, sourceHeight, jrBlendSrcAlphaMerge, false, width, height);
		}
	}

	//mydebug("ATTN: pluginfilter1");

	// and now render border (or border with mask) on top
	if (isEnabledAndWorkingBorder()) {
		if (isEnabledAndWorkingBorderMask()) {
			// mask out first
			//mydebug("ATTN: pluginfilter2");
			jrRenderTextureIntoTexture(outTexrender, texrenderBorderMask, width, height, jrBlendSrcAlphaMask);
		}
		// and now actual border on top (possibly cached to output)
		//jrRenderTextureIntoTexture(outTexrender, texrenderBorder, width, height, jrBlendSrcAlphaMerge);
		//mydebug("ATTN: pluginfilter3");
		jrRenderTextureIntoTexture(outTexrender, texrenderBorder, width, height, jrBlendSrcObsSep);
	}

	if (outTexrender!=NULL) {
		// from temp to screen
		//mydebug("ATTN: pluginfilter4");
		jrRenderTextureIntoTexture(NULL, outTexrender, width, height, jrBlendSrcObsSep);
	}

	//mydebug("ATTN: pluginfilter5");
	//endLinearRgbSpaceMode();

	return true;
}
//---------------------------------------------------------------------------




























//---------------------------------------------------------------------------
bool JrBorder::reloadCustomMaskFile() {
	freeCustomMaskFile();

	if (opt_customMaskFilePath == "") {
		return true;
	}

	gs_image_file_init(&customMaskImage, opt_customMaskFilePath.c_str());
	//
	obs_enter_graphics();
	gs_image_file_init_texture(&customMaskImage);
	obs_leave_graphics();
	//
	customMaskLoaded = customMaskImage.loaded;
	if (customMaskLoaded) {
		customMaskTexture = customMaskImage.texture;
	}
	else {
		return false;
	}

	return true;
}


void JrBorder::freeCustomMaskFile() {
	if (customMaskLoaded) {
		obs_enter_graphics();
		gs_image_file_free(&customMaskImage);
		customMaskTexture = NULL;
		obs_leave_graphics();
		customMaskLoaded = false;
	}
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrBorder::buildCustomMaskAndBorder() {
	// here we want to take our custom mask Texture file we loaded and create an appropriate mask texture with colors DefMaskColorExclude and DefMaskColorInclude

	if (!customMaskLoaded) {
		return;
	}

	// render border mask directly or indirectly?
	gs_texrender_t* borderMaskForBorder = texrenderBorderMask;
	if (opt_maskInside) {
		// temporary place
		borderMaskForBorder = texrenderBorderMaskTemp;
	}

	// mask from file
	std::string techniqueName;
	if (opt_customMaskType == customMaskTypeEnumColor) {
	// make mask by converting white to DefMaskColorInclude and black to DefMaskColorExclude
		techniqueName = "DrawConvertColorToZeroColorAlpha";
	}
	else if (opt_customMaskType==customMaskTypeEnumAlpha) {
		// make mask by converting alpha to DefMaskColorInclude and black to DefMaskColorExclude
		techniqueName = "DrawConvertAlphaToZeroColorAlpha";
	}
	else if (opt_customMaskType==customMaskTypeEnumChromaAlphaLerp) {
		// make mask by converting chroma or white transparent to transparent, and black transparent to mask
		techniqueName = "DrawConvertChromaAlphaToZeroColorAlphaLerped";
	}


	if (opt_customMaskType == customMaskTypeEnumChromaAlphaLerp) {
		// render BORDER MASK using technique
		setEffectsCustomMask();
		jrRenderEffectIntoTextureT(texrenderBorderMask, effectCustomMask, customMaskTexture, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, (techniqueName+"Mask").c_str());
		// now BORDER using different technique
		setEffectsCustomMask();
		if (opt_backTextureEnabled) {
			//jrSetEffectTextureParamByName(effectCustomMask, backTexture, cropScaleStageFinalWidth, cropScaleStageFinalHeight, "imageBackTex");
			jrRenderEffectIntoTextureT(texrenderBorder, effectCustomMask, customMaskTexture, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, (techniqueName + "BorderBackTex").c_str());
		} else {
			jrRenderEffectIntoTextureT(texrenderBorder, effectCustomMask, customMaskTexture, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, (techniqueName + "Border").c_str());
		}
		flagUsingBorder = true;
		flagUsingBorderMask = true;
	}
	else {
		// render BORDER MASK using technique
		setEffectsCustomMask();
		jrRenderEffectIntoTextureT(borderMaskForBorder, effectCustomMask, customMaskTexture, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, techniqueName.c_str());

		// now BORDER generation dynamically from mask using modifiers set in effects
		setEffectsCustomMask();
		if (opt_backTextureEnabled) {
			//jrSetEffectTextureParamByName(effectCustomMask, backTexture, cropScaleStageFinalWidth, cropScaleStageFinalHeight, "imageBackTex");
			jrRenderEffectIntoTexture(texrenderBorder, effectCustomMask, borderMaskForBorder, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, "SuperimposedBorderBackTex");
		}
		else {
			jrRenderEffectIntoTexture(texrenderBorder, effectCustomMask, borderMaskForBorder, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, "SuperimposedBorder");
		}
		// BUT if we want inner mask, we need to recreate mask
		if (opt_maskInside) {
			// there should be a cleaner way to do this
			// we want to remake MASK but now smaller to match inside of border, so we essentially just shrink it down using parameters in SuperimposedBorder
			setEffectsCustomMask();
			jrRenderEffectIntoTexture(texrenderBorderMask, effectCustomMask, borderMaskForBorder, cropScaleStageFinalWidth, cropScaleStageFinalHeight, jrBlendPureCopy, "InnerBorderMaskShrink");
		}
		flagUsingBorder = true;
		flagUsingBorderMask = true;
	}

}


void JrBorder::setEffectsCustomMask() {
	// scale for border generation from mask
	// we want to calculate a shrunken version of mark as if we were scaling it down and superimposing
	if (!effectCustomMask || cropScaleStageFinalWidth <= 0 || cropScaleStageFinalHeight <= 0) {
		return;
	}
	gs_eparam_t *param;

	// border generator options
	struct vec2 border_add_val = {0};
	struct vec2 border_mul_val = {1.0f, 1.0f};
	double borderOffsetX = (double)opt_borderWidth / (double)cropScaleStageFinalWidth;
	double borderOffsetY = (double)opt_borderWidth / (double)cropScaleStageFinalHeight;
	if (borderOffsetX > 0.45) {
		borderOffsetX = 0.45;
	}
	if (borderOffsetY > 0.45) {
		borderOffsetY = 0.45;
	}
	border_add_val.x = -borderOffsetX;
	border_add_val.y = -borderOffsetY;
	border_mul_val.x = 1.0f / (1.0f - borderOffsetX*2.0);
	border_mul_val.y = 1.0f / (1.0f - borderOffsetY*2.0);

	// mask stretching
	struct vec2 add_val = {0};
	struct vec2 mul_val = {1.0f, 1.0f};
	if (!opt_customMaskStretchAspect) {
		struct vec2 source_size;
		struct vec2 mask_size;
		struct vec2 mask_temp;
		float source_aspect;
		float mask_aspect;
		bool size_to_x;
		float fix;
		//
		source_size.x = (float)cropScaleStageFinalWidth;
		source_size.y = (float)cropScaleStageFinalHeight;
		mask_size.x =	(float)gs_texture_get_width(customMaskTexture);
		mask_size.y =	(float)gs_texture_get_height(customMaskTexture);
		//
		source_aspect = source_size.x / source_size.y;
		mask_aspect = mask_size.x / mask_size.y;
		size_to_x = (source_aspect < mask_aspect);
		//
		fix = size_to_x ? (source_size.x / mask_size.x)
				: (source_size.y / mask_size.y);
		//
		vec2_mulf(&mask_size, &mask_size, fix);
		vec2_div(&mul_val, &source_size, &mask_size);
		vec2_mulf(&source_size, &source_size, 0.5f);
		vec2_mulf(&mask_temp, &mask_size, 0.5f);
		vec2_sub(&add_val, &source_size, &mask_temp);
		vec2_neg(&add_val, &add_val);
		vec2_div(&add_val, &add_val, &mask_size);
	}

	// set effect params
	param = gs_effect_get_param_by_name(effectCustomMask, "mul_val");
	gs_effect_set_vec2(param, &mul_val);
	param = gs_effect_get_param_by_name(effectCustomMask, "add_val");
	gs_effect_set_vec2(param, &add_val);
	param = gs_effect_get_param_by_name(effectCustomMask, "border_mul_val");
	gs_effect_set_vec2(param, &border_mul_val);
	param = gs_effect_get_param_by_name(effectCustomMask, "border_add_val");
	gs_effect_set_vec2(param, &border_add_val);
	param = gs_effect_get_param_by_name(effectCustomMask, "color");
	struct vec4 borderColorVec;
	jrazUint32ToRgbaVec(opt_borderColor, borderColorVec);
	gs_effect_set_vec4(param, &borderColorVec);

	vec2 pixel_size;
	pixel_size.x = 1.0f / (float)cropScaleStageFinalWidth;
	pixel_size.y = 1.0f / (float)cropScaleStageFinalHeight;
	param = gs_effect_get_param_by_name(effectCustomMask, "pixel_size");
	gs_effect_set_vec2(param, &pixel_size);

	param = gs_effect_get_param_by_name(effectCustomMask, "colorBlendLerp");
	float colorBlendLerp = (float)opt_colorBlendLerp / 100.0f;
	gs_effect_set_float(param, colorBlendLerp);

	param = gs_effect_get_param_by_name(effectCustomMask, "colorBlendLerpBack");
	float colorBlendLerpBack = (float)opt_colorBlendLerpBack / 100.0f;
	gs_effect_set_float(param, colorBlendLerpBack);

	// inner limits -- no masking outside of this area, which is the original image
	vec4 innerLimits;
	setInnerLimitsOriginalImage(innerLimits);
	param = gs_effect_get_param_by_name(effectCustomMask, "innerLimits");
	gs_effect_set_vec4(param, &innerLimits);

	jrSetEffectTextureParamByName(effectCustomMask, backTexture, cropScaleStageFinalWidth, cropScaleStageFinalHeight, "imageBackTex");
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrBorder::startLinearRgbSpaceMode() {
	/* need linear path for correct alpha blending */
	const bool linear_srgb = true;
	previousLinearRgb = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(linear_srgb);
}

void JrBorder::endLinearRgbSpaceMode() {
	gs_enable_framebuffer_srgb(previousLinearRgb);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrBorder::setInnerLimitsOriginalImage(vec4& innerLimits) {
	// this helps us limit cropping

	if (getIsPluginTypeFilter()) {
		float offx = cropScaleStageOffx;
		float offy = cropScaleStageOffy;

		// kludge
		//--offx;
		//--offy;

		innerLimits.ptr[0] = (float)offx / (float)cropScaleStageFinalWidth;
		innerLimits.ptr[1] = (float)offy / (float)cropScaleStageFinalHeight;
		innerLimits.ptr[2] = (float)(offx + cropScaleStageCoreWidth) / (float)cropScaleStageFinalWidth;
		innerLimits.ptr[3] = (float)(offy + cropScaleStageCoreHeight) / (float)cropScaleStageFinalHeight;
		//mydebug("Source FILTER calcs size = %d x %d vs %f x %f vs %d x %d | %d vs %d (%f, %f)", cropScaleStageFinalWidth, cropScaleStageFinalHeight, shadowedWidth, shadowedHeight, width, height, cropScaleStageCoreWidth, cropScaleStageCoreHeight,offx, offy);
	}
	else {
		float offx = ((float)cropScaleStageFinalWidth - shadowedWidth) / 2.0;
		float offy = ((float)cropScaleStageFinalHeight - shadowedHeight) / 2.0;

		// ATTN: 4/17/23 - attempting to get rid of final line of source at bottom when border offset is 100% but this doesnt seem to be cause of problem
		// but it doesnt seem to be the right way to do it; i think its our source plugin not being tall enough (integer truncation of size?); so try setting this kludge back to 0 once we solve
		// this should be 0 based on our tests with custom mask.. so it's really something else causing the last line to show through on circle

		// kludge
		int offsetKludgeUpperLeft = 2;
		int offsetKludgrLowerRight = 1;

		innerLimits.ptr[0] = (float)(offx-offsetKludgeUpperLeft) / (float)cropScaleStageFinalWidth;
		innerLimits.ptr[1] = (float)(offy-offsetKludgeUpperLeft) / (float)cropScaleStageFinalHeight;
		innerLimits.ptr[2] = (float)(cropScaleStageFinalWidth-(offx-offsetKludgrLowerRight)) / (float)cropScaleStageFinalWidth;
		innerLimits.ptr[3] = (float)(cropScaleStageFinalHeight-(offy-offsetKludgrLowerRight)) / (float)cropScaleStageFinalHeight;
		//mydebug("Source SRC calcs size = %d x %d VS  %d x %d || vs %d x %d vs || %f x %f vs %d %d (%f, %f)", cropScaleStageFinalWidth, cropScaleStageFinalHeight, cropScaleStageCoreWidth, cropScaleStageCoreHeight, cropScaleStageOffx, cropScaleStageOffy, shadowedWidth, shadowedHeight, width, height, offx, offy);
	}
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
float JrBorder::calcAdjustedOffset() {
	// offset
	if (!opt_enable_border) {
		return 0;
	}

	float offsetPixels = calcOffsetPixels();
	return opt_borderWidth - offsetPixels;
}

float JrBorder::calcOffsetPixels() {
	if (opt_customMaskType==customMaskTypeEnumChromaAlphaLerp && opt_borderShape == borderShapeEnumCustom) {
		return 0;
	}

	// ATTN; we seem to get an artifact when this is 100/100 so we are trying to make it slightly smaller
	return (float)(((float)opt_borderOffset / 100.0f) * (float)opt_borderWidth);
	//return (float)(((float)opt_borderOffset / 101.0f) * (float)opt_borderWidth);
}
//---------------------------------------------------------------------------












//---------------------------------------------------------------------------
bool JrBorder::reloadBackTextureFile() {
	freeBackTextureFile();

	if (opt_backTextureFilePath == "") {
		return true;
	}

	gs_image_file_init(&backTextureImage, opt_backTextureFilePath.c_str());
	//
	obs_enter_graphics();
	gs_image_file_init_texture(&backTextureImage);
	obs_leave_graphics();
	//
	backTextureLoaded = backTextureImage.loaded;
	if (backTextureLoaded) {
		backTexture = backTextureImage.texture;
	}
	else {
		return false;
	}

	return true;
}


void JrBorder::freeBackTextureFile() {
	if (backTextureLoaded) {
		obs_enter_graphics();
		gs_image_file_free(&backTextureImage);
		backTexture = NULL;
		obs_leave_graphics();
		backTextureLoaded = false;
	}
}
//---------------------------------------------------------------------------
