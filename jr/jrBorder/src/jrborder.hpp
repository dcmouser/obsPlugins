
#pragma once

#include "plugininfo.hpp"
#include "../../jrcommon/src/jrobsplugin_source.hpp"

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>
#include <obs-module.h>

#include "../../plugins/obs-websocket/lib/obs-websocket-api.h"





//---------------------------------------------------------------------------
// dumb simple data structure
#define DefMaxSceneEntries		30
#define DefMaxSceneNameLen		80
#define DefMaxSceneEntryLineLen	80
#define DefMaxSceneEntryLen		512
struct SplitSceneEntry {
	char sceneName[DefMaxSceneNameLen];
	float splitPos;
	bool splitPosIsAbsolute;
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// settings
#define Setting_EnabledBorder				"enableborder"
#define Setting_EnabledBorder_Text			obs_module_text("Enable border")
#define Setting_EnabledBorder_Def			true
#define Setting_EnabledCropZoom				"enablecropzoom"
#define Setting_EnabledCropZoom_Text			obs_module_text("Enable crop and scale")
#define Setting_EnabledCropZoom_Def			true
//#define Setting_Inner					"inner"
//#define Setting_Inner_Text				obs_module_text("Draw border inside bounds")
//#define Setting_Inner_Def				true
#define Setting_borderStyle				"style"
#define Setting_borderStyle_Text			obs_module_text("Border style")
#define Setting_borderStyle_Def				"Outer"
// see .cpp for Setting_borderStyle_Choices	

#define Setting_borderWidth				"borderWidth"
#define Setting_borderWidth_Text			obs_module_text("Border width (pixels)")
#define Setting_borderWidth_Def				15
#define Setting_borderColor				"borderColor"
#define Setting_borderColor_Text			obs_module_text("Border color")
#define Setting_borderColor_Def				0xFFFFFFFF
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class OptionsDialog;
enum Setting_borderStyle_ChoicesEnum { borderStyleEnumOuter, borderStyleEnumInnerShrink, borderStyleEnumInnerOverlap };
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
class JrBorder : public jrObsPluginSource {
public:
	gs_effect_t *effectCropScale;
	gs_effect_t *effectBorder;
	//
	gs_eparam_t* param_border_src_minx, *param_border_src_miny, *param_border_src_maxx, *param_border_src_maxy;
	gs_eparam_t* param_crop_mulVal, * param_crop_addVal;
	gs_eparam_t* param_border_color;
	gs_eparam_t* param_border_mulVal;
	gs_eparam_t* param_border_addVal;
public:
	bool opt_enable_cropzoom;
	bool opt_enable_border;
	int opt_borderStyle;
	int opt_borderWidth;
	uint32_t opt_borderColor;
	int opt_cropLeft, opt_cropRight, opt_cropTop, opt_cropBottom;
	int opt_scale=0;
	bool flagUseScaleStageForShrinkStyle = true;
	int opt_panX, opt_panY;
	int opt_aspectRatio;
public:
	gs_texrender_t *texrenderSource;
	gs_texrender_t *texrenderPreBorder;
public:
	JrBorder();
	virtual ~JrBorder();
protected:
	bool init();
	void deInitialize();
	bool loadEffects();
protected:
	virtual const char* getPluginName() { return PLUGIN_NAME; };
	virtual const char* getPluginLabel() { return PLUGIN_LABEL; };
	virtual const char* getPluginConfigFileName() { return PLUGIN_CONFIGFILENAME; };
	virtual const char* getPluginOptionsLabel() { return PLUGIN_OPTIONS_LABEL; };
public:
	virtual void onModulePostLoad();
	virtual void onModuleUnload();
public:
	bool isEnabledCropOrZoom() { return opt_enable_cropzoom && (opt_cropLeft != 0 || opt_cropRight != 0 || opt_cropTop != 0 || opt_cropBottom != 0 || opt_scale != 0 || opt_aspectRatio != 0); };
	bool isEnabledCrop() { return opt_enable_cropzoom &&  (opt_cropLeft != 0 || opt_cropRight != 0 || opt_cropTop != 0 || opt_cropBottom != 0); };
	bool isEnabledZoom() { return opt_enable_cropzoom &&  (opt_scale != 0 || opt_aspectRatio != 0) ; };
	bool isEnabledBorder() { return opt_enable_border; };
	bool calcUseScaleStageForThisShrinkStyle() { return (opt_enable_border && opt_borderStyle == borderStyleEnumInnerShrink && flagUseScaleStageForShrinkStyle && isEnabledCropOrZoom()); }
protected:
	void setupEffect(float splitPosition);
public:
	virtual const char* gon_plugin_get_name() { return getPluginName(); };
	virtual void gon_pluginModuleSingletonLoadDoRegistration();
	//
	virtual obs_properties_t* gon_plugin_get_properties();
	static void gon_plugin_get_defaults(obs_data_t* settings);
public:
	virtual void gon_plugin_update( obs_data_t* settings);
	virtual bool gon_plugin_render(gs_effect_t* obsoleteFilterEffect);
public:
	void doRenderEffectCropScale(gs_texrender_t* texSource, gs_texrender_t* texDest, int &renderedWidth, int &renderedHeight);
	void doRenderEffectBorder(gs_texrender_t* texSource, gs_texrender_t* texDest, int &renderedWidth, int &renderedHeight);
	//void setupEffect();
};
