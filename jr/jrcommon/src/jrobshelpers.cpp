#include "jrobshelpers.hpp"

#include <../obs-frontend-api/obs-frontend-api.h>
#include <./util/platform.h>


//---------------------------------------------------------------------------
#include <vector>
#include <string>
#include <algorithm>

//#include <../qt-wrappers.hpp>
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//#define DefJrCustomObsBuild
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
int jrSourceCalculateWidth(obs_source_t* src) {
	//return obs_source_get_base_width(src);
	return obs_source_get_width(src);
}

int jrSourceCalculateHeight(obs_source_t* src) {
	//return obs_source_get_base_height(src);
	return obs_source_get_height(src);
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// for debugging
void jrRgbaDrawPixel(uint32_t *textureData, int ilinesize, int x, int y, int pixelVal) {
	textureData[y * (ilinesize/4) + x] = pixelVal;
}

void jrRgbaDrawRectangle(uint32_t *textureData, int ilinesize, int x1, int y1, int x2, int y2, int pixelVal) {
	for (int iy=y1; iy<=y2; ++iy) {
		jrRgbaDrawPixel(textureData, ilinesize, x1, iy, pixelVal);
		jrRgbaDrawPixel(textureData, ilinesize, x2, iy, pixelVal);
	}
	for (int ix=x1; ix<=x2; ++ix) {
		jrRgbaDrawPixel(textureData, ilinesize, ix, y1, pixelVal);
		jrRgbaDrawPixel(textureData, ilinesize, ix, y2, pixelVal);
	}
}
//---------------------------------------------------------------------------













//---------------------------------------------------------------------------
void jrSetBlendClear(jrBlendClearMode blendClearMode) {
	// see https://learnopengl.com/Advanced-OpenGL/Blending
	gs_reset_blend_state();
	if (blendClearMode == jrBlendOverwriteMerge) {
		// debug mode needs to not clear, but it also doesnt want its partial alpha to show through
		// this blend does NOT dim the colors according to alpha, as we want, so we instead do it in the filter; if we can do it at render time it would be more efficient to not have effect do the extra work of dimming the rgb
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	} else if (blendClearMode == jrBlendDebugOverlay) {
		// debug mode needs to not clear, but it also doesnt want its partial alpha to show through
		// this blend does NOT dim the colors according to alpha, as we want, so we instead do it in the filter; if we can do it at render time it would be more efficient to not have effect do the extra work of dimming the rgb
		//gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
	} else if (blendClearMode == jrBlendPassthroughMerge) {
		// we want to merge onto another texture
		gs_blend_function(GS_BLEND_SRCALPHA, GS_BLEND_INVDSTALPHA);
	} else if (blendClearMode == jrBlendSrcAlphaMerge) {
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
	}

	else if (blendClearMode == jrBlendSrcObsSep) {
		// see obs-source.c
		gs_blend_function_separate(GS_BLEND_SRCALPHA, GS_BLEND_INVSRCALPHA, GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
		/*
		struct vec4 clear_color;
		vec4_zero(&clear_color);
		gs_clear(GS_CLEAR_COLOR, &clear_color, 0.0f, 0);
		*/
	}
	else if (blendClearMode == jrPureCopyNoClear) {
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
	}

	else if (blendClearMode == jrBlendSrcAlphaMask) {
		// we want to mask out stuff what settings should we use? no one cares to document thse values
		//gs_blend_function(GS_BLEND_ONE, GS_BLEND_SRCALPHA);
		gs_blend_function(GS_BLEND_DSTALPHA, GS_BLEND_SRCALPHA);
		//gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
		//gs_blend_function(GS_BLEND_ZERO, );
		//gs_blend_function(GS_BLEND_ZERO, GS_BLEND_ONE);
	} else if (blendClearMode == jrBlendSrcAlphaMask2) {
		// we want to mask out stuff what settings should we use? no one cares to document thse values
		gs_blend_function_separate(GS_BLEND_ZERO, GS_BLEND_SRCALPHA, GS_BLEND_DSTALPHA, GS_BLEND_SRCALPHA);
	}  else if (blendClearMode == jrBlendPureCopy) {
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);
		//gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
	} else {
		// jrBlendClearOverwite gets here
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
	}
	//
	if (blendClearMode == jrBlendClearOverwite || blendClearMode == jrBlendPureCopy) {
		struct vec4 clear_color;
		vec4_zero(&clear_color);
		gs_clear(GS_CLEAR_COLOR, &clear_color, 0.0f, 0);
	}
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrRenderSourceIntoTexture(obs_source_t* source, gs_texrender_t* tex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode) {
	jrRenderSourceIntoTextureAtSizeLoc(source, tex, sourceWidth, sourceHeight, 0, 0, sourceWidth, sourceHeight, blendClearMode, false);
}



// see source_render
void jrRenderSourceOut(obs_source_t* source, uint32_t sourceWidth, uint32_t sourceHeight, int outWidth, int outHeight, jrBlendClearMode blendClearMode) {
	if (sourceWidth == 0 || sourceHeight == 0) {
		return;
	}
	// render it out
	jrRenderSourceIntoTextureAtSizeLoc(source, NULL, sourceWidth, sourceHeight, 0,0, outWidth, outHeight, blendClearMode, false);
}




void jrRenderSourceIntoTextureAtSizeLoc(obs_source_t* source, gs_texrender_t *tex, uint32_t sourceWidth, uint32_t sourceHeight, int x1, int y1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, bool forceResizeToScreen, int tsetwidth, int tsetheight) {
	// render source onto a texture

	//mydebug("myRenderSourceIntoTexture source=(%d,%d) out=(%d,%d).", sourceWidth, sourceHeight, outWidth, outHeight);

	// setup rendering to texture (also sets output size!)
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode, tsetwidth, tsetheight);

	if (tex) {
		// only if TEX is being used as recipient of our drawing do we resize to fill tex, otherwise we are going to screen and do NOT want to do this
		// So for example if we use this gs_ortho, the output will scale to fill the entire screen exactly.. but that's not what we want.  we want the screen to stay as it is, and this output to take extent outWidth on the SCREEN
		// so when rendering to a SCREEN, we do NOT use gs_ortho.. when rendering to a TEXTURE that we want to fill, we do it
		gs_ortho(0.0f, (float)sourceWidth, 0.0f, (float)sourceHeight, -100.0f, 100.0f);
	}

	//offset in so we can debug multiple sources on same screen
	gs_matrix_translate3f((float)x1, (float)y1, 0.0f);

	// do we need to do this? because we cant ask sprite to render at target size
	// maybe only if going to screen
	if (forceResizeToScreen) {
		//gs_ortho(0.0f, (float)sourceWidth, 0.0f, (float)sourceHeight, -100.0f, 100.0f);
		gs_matrix_scale3f((float)outWidth / (float)sourceWidth, (float)outHeight / (float)sourceHeight, 0);
	}

	// RENDER THE SOURCE
	obs_source_video_render(source);

	// restore state and finalize texture
	jrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------























//---------------------------------------------------------------------------
void jrRenderEffectIntoTexture(gs_texrender_t *tex, gs_effect_t* effect, gs_texrender_t *inputTex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode, const char* drawTechnique) {
	// let's try using our new function
	jrRenderEffectIntoTextureAtSizeLoc(tex, effect, inputTex, NULL, sourceWidth, sourceHeight, 0, 0, sourceWidth, sourceHeight, blendClearMode, drawTechnique);
}


void jrRenderEffectIntoTextureT(gs_texrender_t* tex, gs_effect_t* effect, gs_texture_t* obsInputTex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode, const char* drawTechnique) {
	jrRenderEffectIntoTextureAtSizeLoc(tex, effect, NULL, obsInputTex, sourceWidth, sourceHeight, 0, 0, sourceWidth, sourceHeight, blendClearMode, drawTechnique);
}



void jrRenderEffectIntoTextureAtSizeLoc(gs_texrender_t *tex, gs_effect_t* effect, gs_texrender_t *inputTex, gs_texture_t* obsInputTex, uint32_t sourceWidth, uint32_t sourceHeight, int outx1, int outy1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, const char* drawTechnique, int tsetwidth, int tsetheight) {
	// render effect onto texture using an input texture (set effect params before invoking)

	// setup rendering to texture (also sets output size!)
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode, tsetwidth, tsetheight);

	//offset location
	gs_matrix_translate3f((float)outx1, (float)outy1, 0.0f);

	if (inputTex || obsInputTex) {
		// specify the image texture to use when running this effect
		gs_eparam_t* image = gs_effect_get_param_by_name(effect, "image");
		if (!image) {
			mydebug("ERROR jrRenderEffectIntoTexture null image!!");
		}
		if (inputTex && !obsInputTex) {
			obsInputTex = gs_texrender_get_texture(inputTex);
			if (!obsInputTex) {
				mydebug("ERROR jrRenderEffectIntoTexture null obsInputTex1!!");
			}
		}
		if (obsInputTex) {
			gs_effect_set_texture(image, obsInputTex);
		}
	}

	if (tex) {
		// only if TEX is being used as recipient of our drawing do we resize to fill text, otherwise we are going to screen and do NOT want to do this
		// So for example if we use this gs_ortho, the output will scale to fill the entire screen exactly.. but that's not what we want.  we want the screen to stay as it is, and this output to take extent outWidth on the SCREEN
		// so when rendering to a SCREEN, we do NOT use gs_ortho.. when rendering to a TEXTURE that we want to fill, we do it
		gs_ortho(0.0f, (float)outWidth, 0.0f, (float)outHeight, -100.0f, 100.0f);
	}

	// do the drawing
	while (gs_effect_loop(effect, drawTechnique)) {
		gs_draw_sprite(obsInputTex, 0, outWidth, outHeight);
	}

	// restore state and finalize texture
	jrRenderTextureRenderEnd(tex);
}

//---------------------------------------------------------------------------
































//---------------------------------------------------------------------------
void jrRenderConfiguredEffectIntoTextureAtSize(gs_texrender_t *tex, gs_effect_t* effect, int sourceWidth, int sourceHeight, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode, const char* drawTechnique) {
	// render effect onto texture using an input texture (set effect params before invoking)
	// drawTechnique should be "FadeLinear"

	// setup rendering to texture
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode, -1,-1);

	if (tex) {
		// if rendering into texture force output size
		gs_ortho(0.0f, (float)outWidth, 0.0f, (float)outHeight, -100.0f, 100.0f);
	}

	while (gs_effect_loop(effect, drawTechnique)) {
		gs_draw_sprite(NULL, 0, outWidth, outHeight);
	}

	// restore state and finalize texture
	jrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrRenderTextureIntoTexture(gs_texrender_t* tex, gs_texrender_t* srcTexRender, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode) {
	// IMPORTANT!!!!!! 4/14/23
	// IF the existing contents of tex are different size, i think we do not get a proper overwrite merge with blendClearMode and instead get a zerod source
	/*
	bool flagDirectCopy = false;
	if (flagDirectCopy) {
		// simpler way -- but does it work?
		if (tex) {
			// texture to texture
			gs_copy_texture(gs_texrender_get_texture(tex), gs_texrender_get_texture(srcTexRender));
			return;
		} else {
			// to screen??
			// is there a faster way to do this other than using a default effect below?
			// obs_source_draw(gs_texrender_get_texture(srcTexRender), 0, 0, outWidth, outHeight, false);
		}
	}
	*/

	// setup rendering to texture
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode, -1,-1);

	if (tex) {
		// if rendering into texture force output size
		gs_ortho(0.0f, (float)outWidth, 0.0f, (float)outHeight, -100.0f, 100.0f);
	}

	// default effect for rendering
	gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// specify the image texture to use when running this effect
	gs_eparam_t *image = gs_effect_get_param_by_name(effect, "image");
	if (!image) {
		mydebug("ERROR jrRenderTextureIntoTexture null image!!");
	}
	gs_texture_t* obsInputTex = NULL;
	if (srcTexRender) {
		obsInputTex = gs_texrender_get_texture(srcTexRender);
		if (!obsInputTex) {
			mydebug("ERROR jrRenderEffectIntoTexture null obsInputTex2!!");
		}
	}
	if (obsInputTex) {
		gs_effect_set_texture(image, obsInputTex);
	}

	// render it
	while (gs_effect_loop(effect, "Draw")) {
		gs_draw_sprite(NULL, 0, outWidth, outHeight);
	}

	// restore state and finalize texture
	jrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void jrRenderTextureIntoTextureBare(gs_texrender_t* tex, gs_texture* srcTexture, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode) {
	// setup rendering to texture
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode, -1,-1);

	if (tex) {
		// if rendering into texture force output size
		gs_ortho(0.0f, (float)outWidth, 0.0f, (float)outHeight, -100.0f, 100.0f);
	}

	// default effect for rendering
	gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);

	// specify the image texture to use when running this effect
	gs_eparam_t *image = gs_effect_get_param_by_name(effect, "image");
	if (!image) {
		mydebug("ERROR jrRenderTextureIntoTexture null image!!");
	}
	gs_texture_t* obsInputTex = srcTexture;
	//
	if (obsInputTex) {
		gs_effect_set_texture(image, obsInputTex);
	}

	// render it
	while (gs_effect_loop(effect, "Draw")) {
		gs_draw_sprite(NULL, 0, outWidth, outHeight);
	}

	// restore state and finalize texture
	jrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrRenderTextureRenderStart(gs_texrender_t* tex, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode, int tsetwidth, int tsetheight) {

	// save state
	gs_viewport_push();
	gs_projection_push();
	gs_matrix_push();
	gs_blend_state_push();

	if (tex) {
		if (true) {
			// does not clear contents just set it as unrendered
			gs_texrender_reset(tex);
		}
		if (tsetwidth == -1) {
			tsetwidth = outWidth;
		}
		if (tsetheight == -1) {
			tsetheight = outHeight;
		}
		//
		if (!gs_texrender_begin(tex, tsetwidth, tsetheight)) {
			mydebug("ERROR ----> failure in jrRenderTextureRenderStart to gs_texrender_begin %d,%d (%d,%d).", outWidth, outHeight, tsetwidth, tsetheight);
		}
		// these dif sizes let us render into a larger target texture
		//gs_set_viewport(0, 0, tsetwidth, tsetheight);
		// ATTN: 4/16/23 putting this back
		// ATTN: 4/16/23 -- no idea if this is doing anything
		gs_set_viewport(0, 0, tsetwidth, tsetheight);
		if (tsetwidth != outWidth || tsetheight != outHeight) {
			gs_matrix_scale3f((float)outWidth / (float)tsetwidth, (float)outHeight / (float)tsetheight, 0);
		}
	}

	// the save state stuff used to be here

	// blend mode and clear
	jrSetBlendClear(blendClearMode);
}


void jrRenderTextureRenderEnd(gs_texrender_t* tex) {
	// restore state and finalize texture

	if (tex) {
		gs_texrender_end(tex);
	}

	gs_blend_state_pop();
	gs_matrix_pop();
	gs_projection_pop();
	gs_viewport_pop();

}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrAddPropertListChoices(obs_property_t* comboString, const char** choiceList) {
	for (int i = 0;; ++i) {
		if (choiceList[i] == NULL || strcmp(choiceList[i], "") == 0) {
			break;
		}
	obs_property_list_add_string(comboString, choiceList[i],choiceList[i]);
	}
}

int jrPropertListChoiceFind(const char* strval, const char** choiceList, int defaultIndex) {
	for (int i = 0;; ++i) {
		if (choiceList[i] == NULL || strcmp(choiceList[i], "") == 0) {
			break;
		}
		if (strcmp(choiceList[i], strval) == 0) {
			return i;
		}
	}
	// not found
	return defaultIndex;
}


const char* jrStringFromListChoice(int index, const char** choiceList) {
	for (int i = 0;; ++i) {
		if (choiceList[i] == NULL || strcmp(choiceList[i], "") == 0) {
			break;
		}
		if (i==index) {
			return choiceList[i];
		}
	}
	return "";
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
struct add_sources_s
{
	obs_source_t *self;
	std::vector<std::string> source_names;
};

bool add_sources(void *data, obs_source_t *source)
{
	auto &ctx = *(add_sources_s*)data;

	if (source == ctx.self)
		return true;

	uint32_t caps = obs_source_get_output_flags(source);
	if (~caps & OBS_SOURCE_VIDEO)
		return true;

	if (obs_source_is_group(source))
		return true;

	const char *name = obs_source_get_name(source);
	ctx.source_names.push_back(name);
	return true;
}

void property_list_add_sources(obs_property_t *prop, obs_source_t *self)
{
	// scenes, same order as the scene list
	obs_frontend_source_list sceneList = {};
	obs_frontend_get_scenes(&sceneList);
	for (size_t i = 0; i < sceneList.sources.num; i++) {
		obs_source_t *source = sceneList.sources.array[i];
		const char *c_name = obs_source_get_name(source);
		std::string name = obs_module_text("Scene: "); name += c_name;
		obs_property_list_add_string(prop, name.c_str(), c_name);
	}
	obs_frontend_source_list_free(&sceneList);

	// sources, alphabetical order
	add_sources_s ctx;
	ctx.self = self;
	obs_enum_sources(add_sources, &ctx);

	std::sort(ctx.source_names.begin(), ctx.source_names.end());

	for (size_t i=0; i<ctx.source_names.size(); i++) {
		const std::string name = obs_module_text("Source: ") + ctx.source_names[i];
		obs_property_list_add_string(prop, name.c_str(), ctx.source_names[i].c_str());
	}
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------

// Copyright (c) 2014, Jan Winkler <winkler@cs.uni-bremen.de>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Universität Bremen nor the names of its
//       contributors may be used to endorse or promote products derived from
//       this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
/* Author: Jan Winkler */

/*! \brief Convert RGB to HSV color space
  
  Converts a given set of RGB values r,g,b into HSV
  coordinates. The input RGB values are in the range [0, 1], and the
  output HSV values are in the ranges h = [0, 360], and s, v = [0,
  1], respectively.
  
  \param fR Red component, used as input, range: [0, 1]
  \param fG Green component, used as input, range: [0, 1]
  \param fB Blue component, used as input, range: [0, 1]
  \param fH Hue component, used as output, range: [0, 360]
  \param fS Hue component, used as output, range: [0, 1]
  \param fV Hue component, used as output, range: [0, 1]
  
*/


unsigned char MyGetAValue(uint32_t color) { return (color & 0xFF000000) >> 24; };

void jrazUint32ToRgbVec(uint32_t color, struct vec3 &clvec) {
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);

	// convert rgb to hsv
	clvec.x = (float)red / 255.0f;
	clvec.y = (float)green / 255.0f;
	clvec.z = (float)blue / 255.0f;
}

void jrazUint32ToRgbaVec(uint32_t color, struct vec4& clvec) {
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);
	BYTE alpha = MyGetAValue(color);

	// convert rgb to hsv
	clvec.x = (float)red / 255.0f;
	clvec.y = (float)green / 255.0f;
	clvec.z = (float)blue / 255.0f;
	clvec.w = (float)alpha / 255.0f;
}


void jrazUint32ToRgbaVecTestLowAlpha(uint32_t color, struct vec4& clvec) {
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);
	BYTE alpha = MyGetAValue(color);

	// convert rgb to hsv
	clvec.x = (float)red / 255.0f;
	clvec.y = (float)green / 255.0f;
	clvec.z = (float)blue / 255.0f;
	clvec.w = 0.1f;
}

void jrazUint32ToRgbaaVec(uint32_t color, struct vec4& clvec) {
	BYTE red = GetRValue(color);
	BYTE green = GetGValue(color);
	BYTE blue = GetBValue(color);

	// convert rgb to hsv
	clvec.x = (float)red / 255.0f;
	clvec.y = (float)green / 255.0f;
	clvec.z = (float)blue / 255.0f;
	clvec.w = 1.0f;
}


void jrazUint32ToHsvVec(uint32_t color, struct vec3& clvec) {
	// first convert to rgb
	jrazUint32ToRgbVec(color, clvec);
	// now from rgb to hsv
	float fR, fG, fB, fH, fS, fV;
	fR = clvec.x;
	fG = clvec.y;
	fB = clvec.z;
	RGBtoHSV(fR, fG, fB, fH, fS, fV);
	clvec.x = (float)(fH / 360.0f);
	clvec.y = fS;
	clvec.z = fV;
}


void RGBtoHSV(float& fR, float& fG, float fB, float& fH, float& fS, float& fV) {
  float fCMax = max(max(fR, fG), fB);
  float fCMin = min(min(fR, fG), fB);
  float fDelta = fCMax - fCMin;
  
  if (fDelta > 0) {
    if(fCMax == fR) {
      fH = 60 * (float)(fmod(((fG - fB) / fDelta), 6));
    } else if(fCMax == fG) {
      fH = 60 * (((fB - fR) / fDelta) + 2);
    } else if(fCMax == fB) {
      fH = 60 * (((fR - fG) / fDelta) + 4);
    }
    
    if(fCMax > 0) {
      fS = fDelta / fCMax;
    } else {
      fS = 0;
    }
    
    fV = fCMax;
  } else {
    fH = 0;
    fS = 0;
    fV = fCMax;
  }
  
  if(fH < 0) {
    fH = 360 + fH;
  }
}


void jrazFillRgbaVec(vec4& colorvec, float red, float green, float blue, float alpha) {
	colorvec.x = red;
	colorvec.y = green;
	colorvec.z = blue;
	colorvec.w = alpha;
}
//---------------------------------------------------------------------------






















//---------------------------------------------------------------------------
struct SouceVisibilityChangeDataT {
	bool forceVisible;
	JrForceSourceStateEnum forceState;
	const char* targetSourceName;
};

void setSourceVisiblityByName(bool flagAllScenes, const char* targetSourceName, JrForceSourceStateEnum forceState) {
	const bool flagMarryToFirstFine = false;
	// callback used on each scene item
	auto cb = [](obs_scene_t *, obs_sceneitem_t *sceneItem, void *param) {
		SouceVisibilityChangeDataT* forceStructp = reinterpret_cast<SouceVisibilityChangeDataT*>(param);
		OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
		auto sourceName = obs_source_get_name(itemSource);
		//mydebug("in setSourceVisiblityByName compareing source '%s' vs '%s'", sourceName,forceStructp->targetSourceName);
		//auto sourceType = obs_source_get_type(itemSource);
		//auto source_id = obs_source_get_unversioned_id(itemSource);
		if (strcmp(sourceName, forceStructp->targetSourceName)==0) {
			// matching source name
			//mydebug("found source to set vis %s", sourceName);
			bool visible = false;
			switch (forceStructp->forceState) {
				case JrForceSourceStateVisible:
					visible = true;
					break;
				case JrForceSourceStateHidden:
					visible = false;
					break;
				case JrForceSourceStateToggle:
					// toggle based on state of FIRST match we find
					visible = !obs_sceneitem_visible(sceneItem);;
					// sync all future discovery states to state of this one, or toggle them all individually?
					if (flagMarryToFirstFine) {
						if (visible) {
							forceStructp->forceState = JrForceSourceStateVisible;
						}
						else {
							forceStructp->forceState = JrForceSourceStateHidden;
						}
					}
					break;
			}
			// force it
			//mydebug("found source to set vis %s setting vis to %d.", sourceName, (int)visible);
			obs_sceneitem_set_visible(sceneItem, visible);
		}
		return true;
	};


	// helper data structure used to pass info on what to do to the callback
	SouceVisibilityChangeDataT forceStruct{ 0 };
	forceStruct.forceVisible = false;
	forceStruct.forceState = forceState;
	forceStruct.targetSourceName = targetSourceName;

	doRunObsCallbackOnScenes(flagAllScenes, cb, &forceStruct, true, !flagAllScenes);
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
void fixAudioMonitoringInObsSource(OBSSource itemSource) {
	auto monitoringType = obs_source_get_monitoring_type(itemSource);
	if (monitoringType == OBS_MONITORING_TYPE_NONE) {
		return;
	}
	// toggle it to reset it and fix it
	obs_source_set_monitoring_type(itemSource, OBS_MONITORING_TYPE_NONE);
	obs_source_set_monitoring_type(itemSource, monitoringType);

	// debug
	//auto sourceName = obs_source_get_name(itemSource);
	//blog(LOG_WARNING,"In fixAudioMonitoringInObsSource %s.", sourceName);
}


void refreshBrowserSource(OBSSource itemSource) {
	auto source_id = obs_source_get_unversioned_id(itemSource);
	if (strcmp(source_id, "browser_source") == 0) {
		// got a browser source!  to force it to refresh we use this kludge i found in another plugin to oscillate the browser source fps
		auto settings = obs_source_get_settings(itemSource);
		auto fps = obs_data_get_int(settings, "fps");
		if (fps % 2 == 0) {
			++fps;
		} else {
			--fps;
		}
		obs_data_set_int(settings, "fps", fps);
		obs_source_update(itemSource, settings);
		obs_data_release(settings);
		// debug
		//auto sourceName = obs_source_get_name(itemSource);
		//blog(LOG_WARNING,"In refreshBrowserSourcesInScenes %s.", sourceName);
	}
}


void restartMediaSource(OBSSource itemSource) {
	// see https://docs.obsproject.com/reference-sources
	uint32_t outputFlags = obs_source_get_output_flags(itemSource);
	if (outputFlags && OBS_SOURCE_CONTROLLABLE_MEDIA == 0) {
		// not controllable
		return;
	}
	// only restart if playing or ended?
	obs_media_state curMediaState = obs_source_media_get_state(itemSource);
	if (curMediaState == OBS_MEDIA_STATE_PLAYING || curMediaState == OBS_MEDIA_STATE_ENDED) {
		obs_source_media_restart(itemSource);
	}
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------
void doRunObsCallbackOnScene(obs_scene_t* scene, SceneEnumCbType cb, void* datap, bool flagRecurseScenes) {
	
	// enumerate items in scene and run callback on them
	if (!flagRecurseScenes) {
		// just run original callback on all items in scene, nothing special to do
		obs_scene_enum_items(scene, cb, datap);
	}
	else {
		// more complicated, we want to loop through child SceneItems, but if we find a scene, we go inside that (instead of OR IN ADDITION TO) calling callback on it
		auto cbOuter = [](obs_scene_t* parentScene, obs_sceneitem_t* sceneItem, void* param) {
			// this is invoked on every sceneitem source of the scene
			// if the sceneitem is itself a scene, we just recursively call ourselves
			ProxyDataPackT* datapackp = (ProxyDataPackT*)param;
			OBSSource itemSource = obs_sceneitem_get_source(sceneItem);
			auto sourceType = obs_source_get_type(itemSource);
			if (sourceType == OBS_SOURCE_TYPE_SCENE) {
				bool flagRunOnSceneChildrenThemselvesToo = true;
				if (flagRunOnSceneChildrenThemselvesToo) {
					// invoke the original callback on this non-scene child source in ADDITION to recursing into it -- this is the best approach since some cbs want to be told about scenes
					datapackp->childcb(datapackp->scene, sceneItem, datapackp->childdatap);
				}
				// it's a child scene, we want to recurse into it
				obs_scene_t* childScene = obs_scene_from_source(itemSource);
				doRunObsCallbackOnScene(childScene, datapackp->childcb, datapackp->childdatap, true);
			}
			else {
				// invoke the original callback on this non-scene child source
				datapackp->childcb(datapackp->scene, sceneItem, datapackp->childdatap);
			}
			return true;
		};
		// iterate all child items, calling OUR proxy callback, which will call user callback; proxy takes void* to the stored data to know how to invoke childcb
		ProxyDataPackT datapack{ 0 };
		datapack.childdatap = datap;
		datapack.childcb = cb;
		datapack.scene = scene;
		// call enum_scene_items with our outer callback which takes packed data that includes pointer to innercb
		obs_scene_enum_items(scene, cbOuter, &datapack);
	}
}




void doRunObsCallbackOnScenes(bool flagAllScenes, SceneEnumCbType cb, void* datap, bool flagUsePreviewSceneInStudioMode, bool flagRecurseScenes) {
	// force off recurse scenes if we are already visiting all scenes
	if (flagAllScenes) {
		flagRecurseScenes = false;
	}

	if (flagAllScenes) {
		// iterate all scenes 
		obs_frontend_source_list sceneList = {};
		obs_frontend_get_scenes(&sceneList);
		for (size_t i = 0; i < sceneList.sources.num; i++) {
			obs_source_t* sceneSource = sceneList.sources.array[i];
			obs_scene_t* scene = obs_scene_from_source(sceneSource);
			if (scene) {
				doRunObsCallbackOnScene(scene, cb, datap, flagRecurseScenes);
			}
		}
		// free scene list
		obs_frontend_source_list_free(&sceneList);
	} else {
		// just current scene (depends whether we are in preview mode?)
		obs_source_t* sceneSource;
		if (!flagUsePreviewSceneInStudioMode && !obs_frontend_preview_program_mode_active()) {
			sceneSource = obs_frontend_get_current_scene();
		}
		else {
			sceneSource = obs_frontend_get_current_preview_scene();
			if (!sceneSource) {
				sceneSource = obs_frontend_get_current_scene();
			}
		}
		//
		obs_scene_t* scene = obs_scene_from_source(sceneSource);
		if (scene) {
			// enumerate items in scene
			doRunObsCallbackOnScene(scene, cb, datap, flagRecurseScenes);
		}
		// free scene
		obs_source_release(sceneSource);
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// obs internals
struct obs_context_data_start {
	char* name;
	void* data;
	// ...
};

struct obs_source_start {
	struct obs_context_data_start context;
	// ...
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void* jrobsGetVoidPointerToSourceContextDataPluginp(OBSSource &sourcep) {
	// this function does not exist in original obs code, it's a modification that i have made to my obs source


	#ifdef DefJrCustomObsBuild
	if (true) {
		// fast sure way
		return obs_get_source_contextData(sourcep);
	}
	#endif

	// is there a kludgey way we can get this?
	// because these are structs we should be able to access memory offsets directly
	obs_source_t* realsourcerep = obs_source_get_ref(sourcep);

	// see obs-internal.h
	struct obs_context_data_start* contextp = reinterpret_cast<obs_context_data_start*>(realsourcerep);
	void* datap = contextp->data;

	// need to release
	obs_source_release(realsourcerep);

	#ifdef DefJrCustomObsBuild
	if (true) {
		// safety check
		void *datapbackup = obs_get_source_contextData(sourcep);
		//mydebug("Sanity checking kludge memory pointers are %p vs %p.", datap, datapbackup);
		if (datap != datapbackup) {
			mydebug("ERROR: mismatch in raw kludge data pointers!!  %p vs %p.", datap, datapbackup);
		}
	}
	#endif

	return datap;
}
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
void jrDrawTextureClear(gs_texrender_t *texrender, uint32_t cx, uint32_t cy)
{
	jrDrawTextureFillColor(texrender, cx, cy, 0x00000000);
}


void jrDrawTextureFillColor(gs_texrender_t *texrender, uint32_t cx, uint32_t cy, unsigned int colorVal)
{
	gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
	gs_eparam_t *color = gs_effect_get_param_by_name(solid, "color");
	gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

	struct vec4 borderColorVec;
	jrazUint32ToRgbaVec(colorVal, borderColorVec);
	gs_effect_set_vec4(color, &borderColorVec);

	jrRenderEffectIntoTexture(texrender, solid, NULL, cx, cy, jrBlendClearOverwite, "Solid");
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrSetEffectTextureParamByName(gs_effect_t* effect, gs_texture_t* texture, int width, int height, std::string effectImageName) {
	gs_eparam_t* param_image = gs_effect_get_param_by_name(effect, effectImageName.c_str());
	if (!param_image) {
		mydebug("ERROR jrSetEffectTextureWithScale couldnt find effect image param");
		return;
	}
	// set image name
	gs_effect_set_texture(param_image, texture);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
unsigned long jr_os_gettime_ms() { return (unsigned long) (os_gettime_ns() / 1000000L); }
//---------------------------------------------------------------------------
