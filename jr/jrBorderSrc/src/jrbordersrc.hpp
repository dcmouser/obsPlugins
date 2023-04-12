
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
// settings
#define Setting_EnabledBorder				"enableborder"
#define Setting_EnabledBorder_Text			obs_module_text("Enable border")
#define Setting_EnabledBorder_Def			true

#define Setting_borderWidth				"borderWidth"
#define Setting_borderWidth_Text			obs_module_text("Border width (pixels)")
#define Setting_borderWidth_Def				15
#define Setting_borderColor				"borderColor"
#define Setting_borderColor_Text			obs_module_text("Border color")
#define Setting_borderColor_Def				0xFFFFFFFF
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define DefJrBorderHookSignalsAndAutoAdapt		true
#define DefJrBorderShowAdjustButtonInPropsAlways	true
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class OptionsDialog;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void onSceneItemTransformChangeHandle(void* vp, calldata_t* cdatap);
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
class JrBorder : public jrObsPluginSource {
public:
	gs_effect_t *effectBorder;
	//
	gs_eparam_t* param_border_color;
public:
	bool opt_enable_border;
	int opt_borderWidth;
	uint32_t opt_borderColor;
	bool opt_flagWatchSignals;
public:
	struct vec4 vcolor;
	struct vec4 vcolor_srgb;
public:
	gs_texrender_t *texrenderSource;
	gs_texrender_t *texrenderPreBorder;
public:
	// scene item callbacks
	obs_source_t* callbackScene = NULL;
	signal_handler_t* sourceSceneSignalHandlerp = NULL;
public:
	bool needsReadjust = true;
	obs_sceneitem_t* needsReadjustTargetItemp = NULL;
	bool flagFallbackGlobalScan = false;
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
	bool isEnabledBorder() { return opt_enable_border; };
	void setNeedsReadjust(bool val, obs_sceneitem_t* targetitemp);
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
	virtual void registerCallbacksAndHotkeys();
	virtual void unregisterCallbacksAndHotkeys();
	static void ObsFrontendEvent(enum obs_frontend_event event, void* ptr);
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
public:
	void updateSizePosFromSceneItem(obs_sceneitem_t* borderSceneItemp, obs_sceneitem_t* shadowedSceneItemp);
public:
	void doRenderEffectBorder(gs_texrender_t* texSource, gs_texrender_t* texDest, int &renderedWidth, int &renderedHeight);
public:
	void color_source_render_helper(int rwidth, int rheight, struct vec4 *colorVal);
	void color_source_render();
public:
	void sceneChangesSetupSceneCallbacks();
	void releaseSceneCallbacks();
	void sceneCallbackTriggers(obs_sceneitem_t* itemp);
	void rescanCurrentSceneAndAdjustSizePositions(obs_sceneitem_t* itemp);
};
