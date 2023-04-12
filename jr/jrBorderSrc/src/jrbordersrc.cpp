/*
ObsScrenFlip - flips left and right halves of screen (configurable where) so that when you read from preview screen
it looks like you are looking off in the other direction.
*/

#include "pluginInfo.hpp"
//
#include "jrbordersrc.hpp"

// this MUST come after pluginInfo included, as it depends on macros there
#include "../../jrcommon/src/jrobsplugin_source_globalfuncs.hpp"

//
#include "../../jrcommon/src/jrhelpers.hpp"
#include "../../jrcommon/src/jrqthelpers.hpp"
#include "../../jrcommon/src/jrobshelpers.hpp"

#include <obs-module.h>
#include <../obs-frontend-api/obs-frontend-api.h>
#include <obs.h>

#include <string>
#include <vector>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <obs-frontend-api.h>


//---------------------------------------------------------------------------
// forward declarations that are extern "C"
#ifdef __cplusplus
extern "C" {
#endif
	bool OnPropertyButtonClickAdjustNowCallback(obs_properties_t *props, obs_property_t *property, void *data);
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
// global singleton source info
struct obs_source_info SingletonSourcePluginInfoJrBorder;

void JrBorder::gon_pluginModuleSingletonLoadDoRegistration() {
	// set up the global singleton info for this source filter type
	setPluginCallbacks(&SingletonSourcePluginInfoJrBorder);
	//
	SingletonSourcePluginInfoJrBorder.id = PLUGIN_NAME_Source;
	SingletonSourcePluginInfoJrBorder.type = OBS_SOURCE_TYPE_INPUT;
	SingletonSourcePluginInfoJrBorder.version = 1;
	SingletonSourcePluginInfoJrBorder.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_SRGB;
	SingletonSourcePluginInfoJrBorder.icon_type = OBS_ICON_TYPE_COLOR,
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

	if (false) {
		releaseSceneCallbacks();
	}

	if (true) {
		obs_enter_graphics();
		// free stuff in graphics mode?
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
	char *effectPath = obs_module_file("jrbordersrc.effect");
	effectBorder = gs_effect_create_from_file(effectPath, NULL);
	bfree(effectPath);
	if (!effectBorder) {
		mydebug("ERROR loading effectBorder effect.");
		return false;
	}

	param_border_color = gs_effect_get_param_by_name(effectBorder, "border_color");

	return true;
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
obs_properties_t* JrBorder::gon_plugin_get_properties() {
	obs_properties_t *props = obs_properties_create();
	obs_properties_t* propgroup;

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "border", "Enable border", OBS_GROUP_CHECKABLE , propgroup);
	obs_properties_add_int_slider(propgroup, Setting_borderWidth, Setting_borderWidth_Text,1,255,1);
	obs_properties_add_color_alpha(propgroup, Setting_borderColor, Setting_borderColor_Text);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "adjustments", "Targeting size+location", OBS_GROUP_NORMAL , propgroup);
	obs_properties_add_bool(propgroup, "watchSignals", "Hook signals/events and auto-adjust size+pos");

	// manually adjust button
	if (DefJrBorderShowAdjustButtonInPropsAlways || !DefJrBorderHookSignalsAndAutoAdapt) {
		obs_properties_add_button(propgroup, "adjustNow", "Adjust size and position to match subsequent scene item", OnPropertyButtonClickAdjustNowCallback);
	}

	return props;
}


bool OnPropertyButtonClickAdjustNowCallback(obs_properties_t* props, obs_property_t* property, void* data) {
	JrBorder *plugin = (JrBorder*) data;
	//plugin->rescanCurrentSceneAndAdjustSizePositions(NULL);
	plugin->setNeedsReadjust(true, NULL);
	return true;
}


void JrBorder::gon_plugin_get_defaults(obs_data_t* settings) {
	obs_data_set_default_bool(settings, "border", true);
	obs_data_set_default_bool(settings, Setting_EnabledBorder, Setting_EnabledBorder_Def);
	obs_data_set_default_int(settings, Setting_borderWidth, Setting_borderWidth_Def);
	obs_data_set_default_int(settings, Setting_borderColor, Setting_borderColor_Def);
	obs_data_set_default_bool(settings, "watchSignals", DefJrBorderHookSignalsAndAutoAdapt);
}


void JrBorder::gon_plugin_update(obs_data_t* settings) {
	// get options
	//opt_enable_border = obs_data_get_bool(settings, Setting_EnabledBorder);
	opt_enable_border = obs_data_get_bool(settings, "border");
	opt_borderWidth = obs_data_get_int(settings, Setting_borderWidth);
	opt_borderColor = obs_data_get_int(settings, Setting_borderColor);

	// create vector colors
	vec4_from_rgba(&vcolor, opt_borderColor);
	vec4_from_rgba_srgb(&vcolor_srgb, opt_borderColor);

	opt_flagWatchSignals = obs_data_get_bool(settings, "watchSignals");

	// and now make changes based on options changing
	forceUpdatePluginSettingsOnOptionChange();

	// setup callbacks etc and trigger update
	sceneChangesSetupSceneCallbacks();
}
//---------------------------------------------------------------------------






























//---------------------------------------------------------------------------
bool JrBorder::gon_plugin_render(gs_effect_t* obsoleteFilterEffect) {
	// calc settings for scene
	updateSourceProperties();

	if (!effectBorder) {
		// dont render normal source, better to show there is error with blankness
		return false;
	}

	if (needsReadjust) {
		// get any taget item to watch for changes (can be null meaning assume its changed)
		obs_sceneitem_t* targetItemp = needsReadjustTargetItemp;
		// clear
		setNeedsReadjust(false, NULL);
		// scan
		rescanCurrentSceneAndAdjustSizePositions(targetItemp);
	}

	// starting texture size
	gs_texrender_t* nextTextureIn = texrenderSource;
	gs_texrender_t* nextTextureOut = NULL;

	// temp source is input, NULL display is output
	nextTextureIn = texrenderSource;
	nextTextureOut = NULL;

	if (isEnabledBorder()) {
		// add border and render to output
		color_source_render();
		//doRenderEffectBorder(nextTextureIn, nextTextureOut, renderedWidth, renderedHeight);
		nextTextureIn = nextTextureOut;
		nextTextureOut = NULL;
	}

	return true;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrBorder::doRenderEffectBorder(gs_texrender_t* texSource, gs_texrender_t* texDest, int &renderedWidth, int &renderedHeight) {
	// remember source size
	int swidth = renderedWidth;
	int sheight = renderedHeight;

	// set color
	struct vec4 borderColorVec;
	jrazUint32ToRgbaVec(opt_borderColor, borderColorVec);
	gs_effect_set_vec4(param_border_color, &borderColorVec);

	jrRenderEffectIntoTextureAtSizeLoc(texDest, effectBorder, texSource, NULL, swidth, sheight, 0, 0, renderedWidth, renderedHeight, jrBlendSrcAlphaMerge, "Draw");
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrBorder::color_source_render_helper(int rwidth, int rheight, struct vec4 *colorVal) {
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *color = gs_effect_get_param_by_name(solid, "color");
	gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

	// set color
	gs_effect_set_vec4(color, colorVal);

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);

	gs_draw_sprite(0, 0, rwidth, rheight);

	gs_technique_end_pass(tech);
	gs_technique_end(tech);
}


void JrBorder::color_source_render() {
	/* need linear path for correct alpha blending */
	const bool linear_srgb = gs_get_linear_srgb() ||
				 (vcolor.w < 1.0f);

	const bool previous = gs_framebuffer_srgb_enabled();

	gs_enable_framebuffer_srgb(linear_srgb);

	if (linear_srgb)
		color_source_render_helper(width, height, &vcolor_srgb);
	else
		color_source_render_helper(width, height, &vcolor);

	gs_enable_framebuffer_srgb(previous);
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void JrBorder::updateSizePosFromSceneItem(obs_sceneitem_t* borderSceneItemp, obs_sceneitem_t* shadowedSceneItemp) {
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
	int swidth = obs_source_get_width(shadowSourcep) - (inShadowCrop.left + inShadowCrop.right);
	int sheight = obs_source_get_height(shadowSourcep) - (inShadowCrop.top + inShadowCrop.bottom);
	int swidthScaled = swidth * scalex;
	int sheightScaled = sheight * scalex;

	// our size
	width = (float)swidthScaled + opt_borderWidth*2;
	height = (float)sheightScaled + opt_borderWidth*2;

	// resets
	outBorderCrop.left = 0;
	outBorderCrop.right = 0;
	outBorderCrop.top = 0;
	outBorderCrop.bottom = 0;
	outBorderTransform.scale.x = 1.0f;
	outBorderTransform.scale.y = 1.0f;

	int shadowX = inShadowTransform.pos.x;
	int shadowY = inShadowTransform.pos.y;
	//
	// convert to center
	if (false) {
		// this works BUT rotation breaks
		if ((inShadowTransform.alignment & OBS_ALIGN_LEFT) != 0) {
			shadowX += (float)swidthScaled / 2.0f;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_RIGHT) != 0) {
			shadowX -= (float)swidthScaled / 2.0f;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_TOP) != 0) {
			shadowY += (float)sheightScaled / 2.0f;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_BOTTOM) != 0) {
			shadowY -= (float)sheightScaled / 2.0f;
		}
		outBorderTransform.alignment = OBS_ALIGN_CENTER;
	}
	else {
		// match shadow alignment?
		if ((inShadowTransform.alignment & OBS_ALIGN_LEFT) != 0) {
			shadowX -= opt_borderWidth;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_RIGHT) != 0) {
			shadowX += opt_borderWidth;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_TOP) != 0) {
			shadowY -= opt_borderWidth;
		}
		if ((inShadowTransform.alignment & OBS_ALIGN_BOTTOM) != 0) {
			shadowY += opt_borderWidth;
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
	if (memcmp(&outBorderTransformOrig, &outBorderTransform, sizeof(outBorderTransform) != 0) || outBorderTransform.pos.x != outBorderTransformOrig.pos.x || outBorderTransform.pos.y != outBorderTransformOrig.pos.y) {
		obs_sceneitem_set_info(borderSceneItemp, &outBorderTransform);
	}
	if (memcmp(&outBorderCropOrig, &outBorderCrop, sizeof(outBorderCrop) != 0)) {
		obs_sceneitem_set_crop(borderSceneItemp, &outBorderCrop);
	}
	/*
	if (memcmp(&outBorderBoundsOrig, &outBorderBounds, sizeof(outBorderBounds) != 0)) {
		obs_sceneitem_set_bounds(borderSceneItemp, &outBorderBounds);
	}
	*/
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
	setNeedsReadjust(true, NULL);
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
		setNeedsReadjust(true, itemp);
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
		bool triggerOnNext = false;
		obs_sceneitem_t* changeditemp = NULL;
	};
	TIterateStruct iterateStruct;

	// callback used on each scene item
	auto cb = [](obs_scene_t*, obs_sceneitem_t* sceneItem, void* param) {
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
				borderTargetp->updateSizePosFromSceneItem(datap->borderSceneItemp, sceneItem);
			}
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

	doRunObsCallbackOnScene(scenep, cb, &iterateStruct, true);
	obs_source_release(sceneSource);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
void JrBorder::setNeedsReadjust(bool val, obs_sceneitem_t* targetitemp) {
	needsReadjust = val;
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
