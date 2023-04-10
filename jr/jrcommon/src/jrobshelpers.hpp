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

#include <../obs-frontend-api/obs-frontend-api.h>



//---------------------------------------------------------------------------
void jrRgbaDrawPixel(uint32_t* textureData, int ilinesize, int x, int y, int pixelVal);
void jrRgbaDrawRectangle(uint32_t* textureData, int ilinesize, int x1, int y1, int x2, int y2, int pixelVal);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
extern int jrSourceCalculateWidth(obs_source_t* src);
extern int jrSourceCalculateHeight(obs_source_t* src);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
enum jrBlendClearMode {jrBlendClearOverwite, jrBlendClearMerge, jrBlendOverwriteMerge, jrBlendDebugOverlay, jrBlendPassthroughMerge, jrBlendSrcAlphaMerge};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrSetBlendClear(jrBlendClearMode blendClearMode);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderSourceIntoTexture(obs_source_t* source, gs_texrender_t* tex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode);
void jrRenderSourceIntoTextureAtSizeLoc(obs_source_t* source, gs_texrender_t* tex, uint32_t sourceWidth, uint32_t sourceHeight, int x1, int y1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, bool forceResizeToScreen);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderSourceOut(obs_source_t* source, uint32_t sourceWidth, uint32_t sourceHeight, int outWidth, int outHeight, jrBlendClearMode blendClearMode);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderEffectIntoTexture(gs_texrender_t* tex, gs_effect_t* effect, gs_texrender_t* inputTex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode, const char* drawTechnique);
void jrRenderEffectIntoTextureAtSizeLoc(gs_texrender_t *tex, gs_effect_t* effect, gs_texrender_t *inputTex, gs_texture_t* obsInputTex, uint32_t sourceWidth, uint32_t sourceHeight, int x1, int y1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, const char* drawTechnique);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderConfiguredEffectIntoTextureAtSize(gs_texrender_t* tex, gs_effect_t* effect, int sourceWidth, int sourceHeight, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode, const char* drawTechnique);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderTextureIntoTexture(gs_texrender_t* tex, gs_texrender_t* srcTexRender, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrAddPropertListChoices(obs_property_t* comboString, const char** choiceList);
int jrPropertListChoiceFind(const char* strval, const char** choiceList, int defaultIndex);
const char* jrStringFromListChoice(int index, const char** choiceList);
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrRenderTextureRenderStart(gs_texrender_t* tex, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode);
void JrRenderTextureRenderEnd(gs_texrender_t* tex);
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
#ifndef PLUGIN_LABEL
	#define do_log(level, format, ...) blog(level, "[UNKNOWNJRPLUGIN] " format, ##__VA_ARGS__)
#else
	#define do_log(level, format, ...) blog(level, "[" ## PLUGIN_LABEL ## "] " format, ##__VA_ARGS__)
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
	void property_list_add_sources(obs_property_t *prop, obs_source_t *self);
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void jrazUint32ToRgbVec(uint32_t color, struct vec3& clvec);
void jrazUint32ToHsvVec(uint32_t color, struct vec3& clvec);
void jrazUint32ToRgbaVec(uint32_t color, struct vec4& clvec);
void jrazUint32ToRgba1Vec(uint32_t color, struct vec4& clvec);
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





