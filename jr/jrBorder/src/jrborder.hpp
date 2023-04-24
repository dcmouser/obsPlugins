
#pragma once

#include "plugininfo.hpp"
#include "../../jrcommon/src/jrobsplugin_source.hpp"

#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>
#include <obs-module.h>
#include <graphics/image-file.h>


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
#define DefJrBorderMaxBorders		4
//
#define DefMaskColorExclude 0x00000000
#define DefMaskColorInclude 0xFF000000
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// settings
#define Setting_EnabledBorder				"enableborder"
#define Setting_EnabledBorder_Text			obs_module_text("Enable border")
#define Setting_EnabledBorder_Def			true
#define Setting_EnabledCropZoom				"enablecropzoom"
#define Setting_EnabledCropZoom_Text			obs_module_text("Enable crop and scale")
#define Setting_EnabledCropZoom_Def			true
#define Setting_borderStyle				"style"
#define Setting_borderStyle_Text			obs_module_text("Constrain size or grow?")
#define Setting_borderStyle_Def				"grow"
// see .cpp for Setting_borderStyle_Choices	

#define Setting_borderWidth				"borderWidth"
#define Setting_borderWidth_Text			obs_module_text("Border width (pixels)")
#define Setting_borderWidth_Def				15
#define Setting_borderColor				"borderColor"
#define Setting_borderColor_Text			obs_module_text("Border color")
#define Setting_borderColor_Def				0xFFFFFFFF
#define Setting_borderColor2_Def			0x990000FF
#define Setting_borderColor3_Def			0x99FF0000
#define Setting_borderColor4_Def			0x9900FF00
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// from obs mask filter code
#define SettingCustomMaskType           "customMaslType"
#define TEXT_PATH_IMAGES		obs_module_text("BrowsePath.Images")
#define TEXT_PATH_ALL_FILES		obs_module_text("BrowsePath.AllFiles")
#define IMAGE_FILTER_EXTENSIONS		" (*.bmp *.jpg *.jpeg *.tga *.gif *.png)"
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
#define DefJrBorderHookSignalsAndAutoAdapt		true
#define DefJrBorderShowAdjustButtonInPropsAlways	true
#define DefJrBorderSafteyKludgeCheckVal			1234567890L
#define DefJrBorderMaxBorders				4
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
#define blog(level, msg, ...) blog(level, "[" PLUGIN_NAME "] " msg, ##__VA_ARGS__)
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class OptionsDialog;
enum Setting_borderStyle_ChoicesEnum { borderStyleEnumConstrain, borderStyleEnumGrow };
enum Setting_borderShape_ChoicesEnum { borderShapeEnumRectangle, borderShapeEnumCircle, borderShapeEnumCustom };
enum Setting_customMaskType_ChoicesEnum { customMaskTypeEnumColor,  customMaskTypeEnumAlpha, customMaskTypeEnumChromaAlphaLerp};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void onSceneItemTransformChangeHandle(void* vp, calldata_t* cdatap);
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
class JrBorder : public jrObsPluginSource {
public:
	long safetyCheckVal = 0;
public:
	gs_effect_t* effectCropScale=NULL;
	gs_effect_t* effectBorder=NULL;
	gs_effect_t* effectCustomMask=NULL;
	//
	gs_eparam_t* param_border_src_minx, *param_border_src_miny, *param_border_src_maxx, *param_border_src_maxy;
	gs_eparam_t* param_crop_mulVal, * param_crop_addVal;
	gs_eparam_t* param_border_color;
	gs_eparam_t* param_border_mulVal;
	gs_eparam_t* param_border_addVal;
	gs_eparam_t* param_borderPatternCount;
	gs_eparam_t* param_color1;
	gs_eparam_t* param_color2;
	gs_eparam_t* param_color3;
	gs_eparam_t* param_color4;
	gs_eparam_t* param_borders[DefJrBorderMaxBorders];
	gs_eparam_t* param_innerBorderEdge;
	//
	gs_eparam_t* param_colorMarkExclude;
	gs_eparam_t* param_colorMarkInclude;
	//
	gs_eparam_t* param_circularMaskDist;
	//
	gs_eparam_t* param_innerLimits;
	//
	gs_eparam_t* param_colorBlendLerpBack;
public:
	bool opt_enable_cropzoom;
	bool opt_enable_border;
	int opt_borderStyle;
	int opt_borderShape;
	int opt_borderWidth;
	int opt_borderOffset;
	uint32_t opt_borderColor;
	int opt_colorBlendLerp;
	int opt_colorBlendLerpBack;
	uint32_t opt_borderColor2, opt_borderColor3, opt_borderColor4;
	bool opt_customColorSyntax;
	std::string opt_borderFormat;
	//
	int opt_cropLeft, opt_cropRight, opt_cropTop, opt_cropBottom;
	int opt_scale=0;
	bool flagUseScaleStageForShrinkStyle = true;
	int opt_panX, opt_panY;
	int opt_aspectRatio;
	bool opt_flagWatchSignals;
	bool opt_maskInside;
	bool opt_customMaskStretchAspect = true;
	//
	Setting_customMaskType_ChoicesEnum opt_customMaskType;
	std::string opt_customMaskFilePath;
	//
	bool opt_backTextureEnabled;
	std::string opt_backTextureFilePath;
public:
	bool previousLinearRgb = false;
public:
	int cachedBorderWidth = -1;
	int cachedBorderHeight = -1;
public:
	struct vec2 cropScaleStage_offVal { 0 };
	struct vec2 cropScaleStage_mulVal { 0 };
	int cropScaleStageOffx;
	int cropScaleStageOffy;
	int cropScaleStageCoreWidth;
	int cropScaleStageCoreHeight;
	int cropScaleStageFinalWidth;
	int cropScaleStageFinalHeight;
	float ourSourceScaleX = 1.0;
	float ourSourceScaleY = 1.0;
public:
	gs_texrender_t *texrenderSource=NULL;
	gs_texrender_t *texrenderBorderMask=NULL;
	gs_texrender_t *texrenderBorder=NULL;
	gs_texrender_t *texrenderTemp=NULL;
	gs_texrender_t *texrenderCustomMask=NULL;
	gs_texrender_t *texrenderBorderMaskTemp = NULL;
	gs_texture_t *customMaskTexture=NULL;
	gs_texture_t *backTexture=NULL;
	gs_image_file_t customMaskImage;
	gs_image_file_t backTextureImage;
	bool flagUsingBorderMask = false;
	bool flagUsingBorder = false;
	bool customMaskLoaded = false;
	bool backTextureLoaded = false;
public:
	// scene item callbacks
	obs_source_t* callbackScene = NULL;
	signal_handler_t* sourceSceneSignalHandlerp = NULL;
public:
	bool needsShadowSourceReadjust = true;
	obs_sceneitem_t* needsReadjustTargetItemp = NULL;
	bool flagFallbackGlobalScan = false;
public:
	float shadowedWidth, shadowedHeight;
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
	bool isEnabledCropOrZoom() { return getIsPluginTypeFilter() && ( (opt_enable_cropzoom && (opt_cropLeft != 0 || opt_cropRight != 0 || opt_cropTop != 0 || opt_cropBottom != 0 || opt_scale != 0 || opt_aspectRatio != 0)) || opt_borderStyle == borderStyleEnumConstrain); };
	bool isEnabledCrop() { return getIsPluginTypeFilter() && opt_enable_cropzoom &&  (opt_cropLeft != 0 || opt_cropRight != 0 || opt_cropTop != 0 || opt_cropBottom != 0); };
	bool isEnabledZoom() { return getIsPluginTypeFilter() && opt_enable_cropzoom &&  (opt_scale != 0 || opt_aspectRatio != 0) ; };
	bool isEnabledBorder() { return opt_enable_border; };
	bool isEnabledAndWorkingBorder() { return isEnabledBorder() && texrenderBorder && flagUsingBorder; };
	bool isEnabledAndWorkingBorderMask() { return flagUsingBorderMask && texrenderBorderMask; };
public:
	virtual const char* gon_plugin_get_name() { return getPluginName(); };
	virtual void gon_pluginModuleSingletonLoadDoRegistration();
	void gon_pluginModuleSingletonLoadDoRegistration_Source();
	void gon_pluginModuleSingletonLoadDoRegistration_Filter();
	//
	virtual obs_properties_t* gon_plugin_get_properties();
	static void gon_plugin_get_defaults(obs_data_t* settings);
public:
	void doPrecalcScaleZoomBorderStuff(int swidth, int sheight);
	void doPrecalcCropScale(int swidth, int sheight);
	void doSetEffectParamsBorder(int swidth, int sheight);
	void doSetEffectParamsCropScale();
public:
	virtual void gon_plugin_update( obs_data_t* settings);
	virtual bool gon_plugin_render(gs_effect_t* obsoleteFilterEffect);
public:
	bool doRenderPluginFilter();
	bool doRenderPluginSource();
public:
	void doRenderEffectCropScale(gs_texrender_t* texDest, gs_texrender_t* texSource, int swidth, int sheight);
	float calcAdjustedOffset();
	float calcOffsetPixels();
public:
	void setNeedsShadowSourceReadjust(bool val, obs_sceneitem_t* targetitemp);
	bool passesSafetySanityCheck() { return (safetyCheckVal == DefJrBorderSafteyKludgeCheckVal); };
public:
	virtual void registerCallbacksAndHotkeys();
	virtual void unregisterCallbacksAndHotkeys();
	static void ObsFrontendEvent(enum obs_frontend_event event, void* ptr);
	virtual void handleObsFrontendEvent(enum obs_frontend_event event);
public:
	void sceneChangesSetupSceneCallbacks();
	void releaseSceneCallbacks();
	void sceneCallbackTriggers(obs_sceneitem_t* itemp);
	void rescanCurrentSceneAndAdjustSizePositions(obs_sceneitem_t* itemp);
public:
	void updateSizePosFromSceneItem(obs_sceneitem_t* borderSceneItemp, obs_sceneitem_t* shadowedSceneItemp);
public:
	bool isBorderTextureCacheValid(int swidth, int sheight) { return (swidth == cachedBorderWidth && sheight == cachedBorderHeight); };
	void markBorderTextureCacheInvalid() { cachedBorderWidth = cachedBorderHeight = -1; };
	bool updateBorderComputationsCacheAndBorderTexture(int swidth, int sheight);
public:
	bool reloadCustomMaskFile();
	void freeCustomMaskFile();
	void buildCustomMaskAndBorder();
	void setEffectsCustomMask();
public:
	void startLinearRgbSpaceMode();
	void endLinearRgbSpaceMode();
public:
	void setInnerLimitsOriginalImage(vec4 &innerLimits);
public:
	virtual void setupOptionsDialog() {;};
public:
	bool reloadBackTextureFile();
	void freeBackTextureFile();
public:
	void forceUpdatePluginSettingsOnOptionChange() { markBorderTextureCacheInvalid(); }
};
