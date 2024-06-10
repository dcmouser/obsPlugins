#pragma once

#include <obs.hpp>

#include <windows.h>
#include <ctime>
//
// obs
#include <obs-module.h>
//
#include <graphics/matrix4.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>

#include <string>


#include <../obs-frontend-api/obs-frontend-api.h>


//---------------------------------------------------------------------------
// ATTN: yikes to have to copy this from texture-render.c
struct gs_texture_render {
	gs_texture_t *target, *prev_target;
	gs_zstencil_t *zs, *prev_zs;
	enum gs_color_space prev_space;

	uint32_t cx, cy;

	enum gs_color_format format;
	enum gs_zstencil_format zsformat;

	bool rendered;
};
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
void jrRgbaDrawPixel(uint32_t* textureData, int ilinesize, int x, int y, int pixelVal);
void jrRgbaDrawRectangle(uint32_t* textureData, int ilinesize, int x1, int y1, int x2, int y2, int pixelVal);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
extern int jrSourceCalculateWidth(obs_source_t* src);
extern int jrSourceCalculateHeight(obs_source_t* src);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
enum jrBlendClearMode {jrBlendClearOverwite, jrBlendClearMerge, jrBlendOverwriteMerge, jrBlendDebugOverlay, jrBlendPassthroughMerge, jrBlendSrcAlphaMerge, jrBlendSrcAlphaMask, jrBlendSrcAlphaMask2, jrBlendPureCopy, jrBlendSrcObsSep, jrPureCopyNoClear};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrSetBlendClear(jrBlendClearMode blendClearMode);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderSourceIntoTexture(obs_source_t* source, gs_texrender_t* tex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode);
void jrRenderSourceIntoTextureAtSizeLoc(obs_source_t* source, gs_texrender_t* tex, uint32_t sourceWidth, uint32_t sourceHeight, int x1, int y1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, bool forceResizeToScreen, int tsetwidth=-1, int tsetheight=-1);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderSourceOut(obs_source_t* source, uint32_t sourceWidth, uint32_t sourceHeight, int outWidth, int outHeight, jrBlendClearMode blendClearMode);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderEffectIntoTexture(gs_texrender_t* tex, gs_effect_t* effect, gs_texrender_t* inputTex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode, const char* drawTechnique);
void jrRenderEffectIntoTextureT(gs_texrender_t* tex, gs_effect_t* effect, gs_texture_t* obsInputTex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode, const char* drawTechnique);
void jrRenderEffectIntoTextureAtSizeLoc(gs_texrender_t* tex, gs_effect_t* effect, gs_texrender_t* inputTex, gs_texture_t* obsInputTex, uint32_t sourceWidth, uint32_t sourceHeight, int outx1, int outy1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, const char* drawTechnique, int tsetwidth=-1, int tsetheight=-1);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderConfiguredEffectIntoTextureAtSize(gs_texrender_t* tex, gs_effect_t* effect, int sourceWidth, int sourceHeight, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode, const char* drawTechnique);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderTextureIntoTexture(gs_texrender_t* tex, gs_texrender_t* srcTexRender, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode);
void jrRenderTextureIntoTextureBare(gs_texrender_t* tex, gs_texture* srcTexture, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrAddPropertListChoices(obs_property_t* comboString, const char** choiceList);
int jrPropertListChoiceFind(const char* strval, const char** choiceList, int defaultIndex);
const char* jrStringFromListChoice(int index, const char** choiceList);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrRenderTextureRenderStart(gs_texrender_t* tex, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode, int tsetwidth, int tsetheight);
void jrRenderTextureRenderEnd(gs_texrender_t* tex);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrDrawTextureClear(gs_texrender_t* texrender, uint32_t cx, uint32_t cy);
void jrDrawTextureFillColor(gs_texrender_t* texrender, uint32_t cx, uint32_t cy, unsigned int colorVal);
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
#ifndef PLUGIN_LABEL
	#define do_log(level, format, ...) blog(level, "[UNKNOWNJRPLUGIN] " format, ##__VA_ARGS__)
#else
	//#define do_log(level, format, ...) blog(level, "[" ## PLUGIN_LABEL ## "] " format, ##__VA_ARGS__)
	#define do_log(level, format, ...) blog(level, "[" PLUGIN_LABEL "] " format, ##__VA_ARGS__)
#endif

#define warn(format, ...) do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG, format, ##__VA_ARGS__)
#define obserror(format, ...) do_log(LOG_ERROR, format, ##__VA_ARGS__)
//
#define mydebug(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
	void property_list_add_sources(obs_property_t *prop, obs_source_t *self, int filterBySourceType=-1);
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void jrazUint32ToRgbVec(uint32_t color, struct vec3& clvec);
void jrazUint32ToHsvVec(uint32_t color, struct vec3& clvec);
void jrazUint32ToRgbaVec(uint32_t color, struct vec4& clvec);
void jrazUint32ToRgba1Vec(uint32_t color, struct vec4& clvec);
void jrazUint32ToRgbaVecTestLowAlpha(uint32_t color, struct vec4& clvec);
void RGBtoHSV(float& fR, float& fG, float fB, float& fH, float& fS, float& fV);
void jrazFillRgbaVec(vec4& colorvec, float red, float green, float blue, float alpha);
unsigned char MyGetAValue(uint32_t color);
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
enum JrForceSourceStateEnum {JrForceSourceStateVisible, JrForceSourceStateHidden, JrForceSourceStateToggle};
void setSourceVisiblityByName(bool flagAllScenes, const char* targetSourceName, JrForceSourceStateEnum forceState);
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void fixAudioMonitoringInObsSource(OBSSource itemSource);
void refreshBrowserSource(OBSSource itemSource);
void restartMediaSource(OBSSource itemSource);
void sendKeyToBrowserSource(OBSSource itemSource, void* param);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// templated function to help with callbacks; we don't really use different type T but its a good way to avoid knowing function signature
typedef bool(*SceneEnumCbType)(obs_scene_t* , obs_sceneitem_t* , void* );
void doRunObsCallbackOnScene(obs_scene_t* scene, SceneEnumCbType cb, void* datap, bool flagRecurseScenes);
void doRunObsCallbackOnScenes(bool flagAllScenes, SceneEnumCbType cb, void* datap, bool flagUsePreviewSceneInStudioMode, bool flagRecurseScenes);
struct ProxyDataPackT {
	void* childdatap;
	SceneEnumCbType childcb;
	obs_scene_t* scene;
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void* jrobsGetVoidPointerToSourceContextDataPluginp(OBSSource& sourcep);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrSetEffectTextureParamByName(gs_effect_t* effect, gs_texture_t* texture, int width, int height, std::string effectImageName);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
unsigned long jr_os_gettime_ms();
//---------------------------------------------------------------------------
