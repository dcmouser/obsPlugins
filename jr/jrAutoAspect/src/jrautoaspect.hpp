#pragma once

#include "plugininfo.hpp"
#include "../../jrcommon/src/jrobsplugin_source.hpp"

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>
#include <obs-module.h>
#include <graphics/image-file.h>

#include <windows.h>

#include "../../plugins/obs-websocket/lib/obs-websocket-api.h"









//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
enum JrOrientationMode { JrOrientationModeEnum_Portrait, JrOrientationModeEnum_Landscape, JrOrientationModeEnum_Auto };
#define DefResizeFactorForScanningX 10
#define DefResizeFactorForScanningY 10
#define DefUint32PixelColorBlank1 0xFF000000
#define DefUint32PixelColorBlank2 0x00000000
#define DefUint32PixelColorBlank3 0xFF101010
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class OptionsDialog;
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
class JrAutoAspect : public jrObsPluginSource {
public:
	gs_effect_t* effectCropScale=NULL;
	//
	gs_eparam_t* param_crop_addVal;
	gs_eparam_t* param_crop_mulVal;
public:
	JrOrientationMode opt_orientationMode;
	int opt_cropLeftp, opt_cropRightp, opt_cropTopp, opt_cropBottomp;
	int opt_cropLeftl, opt_cropRightl, opt_cropTopl, opt_cropBottoml;
	int opt_maxWidth, opt_maxHeight;
	unsigned long opt_recheckEveryMs;
public:
	HANDLE pluginMutex = 0;
public:
	gs_texrender_t *texrenderSource=NULL;
	gs_texrender_t *texrenderTemp=NULL;
	gs_texrender_t *texrenderStaging=NULL;
	gs_stagesurf_t *stagingSurface = NULL;
protected:
	uint8_t* stagingData = NULL;
	int stagingLinesize = 0;
	int allocatedStageWidth = 0;
	int allocatedStageHeight = 0;
protected:
	JrOrientationMode orientationMode = JrOrientationModeEnum_Auto;
	vec2 cropAddVal;
	vec2 cropMulVal;
	float cropScale;
	float renderedWidth, renderedHeight;
public:
	int cachedSourceWidth = -1;
	int cachedSourceHeight = -1;
	bool flagNeedsRecalculation = true;
	unsigned long lastCheckTimeMs = 0;
public:
	JrAutoAspect();
	virtual ~JrAutoAspect();
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
	virtual void setupOptionsDialog() {;};
public:
	virtual void onModulePostLoad();
	virtual void onModuleUnload();
public:
	virtual void gon_pluginModuleSingletonLoadDoRegistration();
	void gon_pluginModuleSingletonLoadDoRegistration_Filter();
	virtual obs_properties_t* gon_plugin_get_properties();
	static void gon_plugin_get_defaults(obs_data_t* settings);
	virtual const char* gon_plugin_get_name() { return getPluginName(); };
public:
	virtual void registerCallbacksAndHotkeys();
	virtual void unregisterCallbacksAndHotkeys();
	static void ObsFrontendEvent(enum obs_frontend_event event, void* ptr);
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);

public:
	virtual void gon_plugin_update( obs_data_t* settings);
	virtual bool gon_plugin_render(gs_effect_t* obsoleteFilterEffect);
public:
	bool doRenderPluginFilter();
public:
	void doRenderEffectCropScale(gs_texrender_t* texDest, gs_texrender_t* texSource, int swidth, int sheight);
	void doSetEffectParamsCropScale();
public:
	void forceUpdatePluginSettingsOnOptionChange() { setNeedsRecalculation(true); }
public:
	void recalculateAspectRatioChoice();
	void setCalculationsBasedOnOrientation();
	JrOrientationMode guessOrientation();
	void setNeedsRecalculation(bool val) { flagNeedsRecalculation = val; }
public:
	uint32_t getPixelFromStagingData(uint32_t* data, int x, int y, int dataLineSize);
	bool isPixelBlank(uint32_t pixel);
	inline int CompPointOffset(int x, int y, int stageWidth) { return stageWidth * y + x; }
	inline int CompRgbaByteOffset(int x, int y, int linesize) { return y * linesize + x * 4; };
	void reallocateStagingSurfaceAndData(int swidth, int sheight);
	void freeStagingSurfaceAndData();
};
