#pragma once

//---------------------------------------------------------------------------
#include <obs-module.h>
#include <../libobs/util/base.h>
//
#include "pluginInfo.h"
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#define do_log(level, format, ...) blog(level, "[" ## DefMyPluginLabel ## "] " format, ##__VA_ARGS__)
//
#define warn(format, ...) do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)
#define debug(format, ...) do_log(LOG_DEBUG, format, ##__VA_ARGS__)
#define obserror(format, ...) do_log(LOG_ERROR, format, ##__VA_ARGS__)
//
#define mydebug(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)
//#define mydebug(format, ...) 
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
enum jrBlendClearMode {jrBlendClearOverwite, jrBlendClearMerge, jrBlendOverwriteMerge, jrBlendDebugOverlay, jrBlendPassthroughMerge};
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
void jrRenderEffectIntoTexture(gs_texrender_t* tex, gs_effect_t* effect, gs_texrender_t* inputTex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode, char* drawTechnique);
void jrRenderEffectIntoTextureAtSizeLoc(gs_texrender_t *tex, gs_effect_t* effect, gs_texrender_t *inputTex, gs_texture_t* obsInputTex, uint32_t sourceWidth, uint32_t sourceHeight, int x1, int y1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, char* drawTechnique);
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void jrRenderConfiguredEffectIntoTextureAtSize(gs_texrender_t* tex, gs_effect_t* effect, int sourceWidth, int sourceHeight, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode, char* drawTechnique);
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
