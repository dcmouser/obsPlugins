#include "obsHelpers.h"
#include "jrPluginDefs.h"





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
	} else {
		gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
	}
	//
	if (blendClearMode == jrBlendClearOverwite) {
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




void jrRenderSourceIntoTextureAtSizeLoc(obs_source_t* source, gs_texrender_t *tex, uint32_t sourceWidth, uint32_t sourceHeight, int x1, int y1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, bool forceResizeToScreen) {
	// render source onto a texture

	//mydebug("myRenderSourceIntoTexture source=(%d,%d) out=(%d,%d).", sourceWidth, sourceHeight, outWidth, outHeight);

	// setup rendering to texture
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode);

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
	JrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------























//---------------------------------------------------------------------------
void jrRenderEffectIntoTexture(gs_texrender_t *tex, gs_effect_t* effect, gs_texrender_t *inputTex, uint32_t sourceWidth, uint32_t sourceHeight, jrBlendClearMode blendClearMode, char* drawTechnique) {
	// let's try using our new function
	jrRenderEffectIntoTextureAtSizeLoc(tex, effect, inputTex, NULL, sourceWidth, sourceHeight, 0, 0, sourceWidth, sourceHeight, blendClearMode, drawTechnique);
	return;
}




void jrRenderEffectIntoTextureAtSizeLoc(gs_texrender_t *tex, gs_effect_t* effect, gs_texrender_t *inputTex, gs_texture_t* obsInputTex, uint32_t sourceWidth, uint32_t sourceHeight, int outx1, int outy1, int outWidth, int outHeight, jrBlendClearMode blendClearMode, char* drawTechnique) {
	// render effect onto texture using an input texture (set effect params before invoking)

	// setup rendering to texture
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode);

	//offset location
	gs_matrix_translate3f((float)outx1, (float)outy1, 0.0f);

	// specify the image texture to use when running this effect
	gs_eparam_t *image = gs_effect_get_param_by_name(effect, "image");
	if (!image) {
		mydebug("ERROR jrRenderEffectIntoTexture null image!!");
	}
	if (inputTex && !obsInputTex) {
		obsInputTex = gs_texrender_get_texture(inputTex);
		if (!obsInputTex) {
			mydebug("ERROR jrRenderEffectIntoTexture null obsInputTex!!");
		}
	}
	if (obsInputTex) {
		gs_effect_set_texture(image, obsInputTex);
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
	JrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------
































//---------------------------------------------------------------------------
void jrRenderConfiguredEffectIntoTextureAtSize(gs_texrender_t *tex, gs_effect_t* effect, int sourceWidth, int sourceHeight, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode, char* drawTechnique) {
	// render effect onto texture using an input texture (set effect params before invoking)
	// drawTechnique should be "FadeLinear"


	// setup rendering to texture
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode);

	if (tex) {
		// if rendering into texture force output size
		gs_ortho(0.0f, (float)outWidth, 0.0f, (float)outHeight, -100.0f, 100.0f);
	}

	while (gs_effect_loop(effect, drawTechnique)) {
		gs_draw_sprite(NULL, 0, outWidth, outHeight);
	}

	// restore state and finalize texture
	JrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void jrRenderTextureIntoTexture(gs_texrender_t* tex, gs_texrender_t* srcTexRender, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode) {
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

	// setup rendering to texture
	jrRenderTextureRenderStart(tex, outWidth, outHeight, blendClearMode);

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
			mydebug("ERROR jrRenderEffectIntoTexture null obsInputTex!!");
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
	JrRenderTextureRenderEnd(tex);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void jrRenderTextureRenderStart(gs_texrender_t* tex, uint32_t outWidth, uint32_t outHeight, jrBlendClearMode blendClearMode) {
	if (tex) {
		if (true) {
			gs_texrender_reset(tex);
		}
		if (!gs_texrender_begin(tex, outWidth, outHeight)) {
			mydebug("ERROR ----> failure in jrRenderTextureRenderStart to gs_texrender_begin %d,%d.", outWidth, outHeight);
			return;
		}
	}

	// save state
	gs_viewport_push();
	gs_projection_push();
	gs_matrix_push();
	gs_blend_state_push();

	// blend mode and clear
	jrSetBlendClear(blendClearMode);
}


void JrRenderTextureRenderEnd(gs_texrender_t* tex) {
	// restore state and finalize texture
	gs_blend_state_pop();
	gs_matrix_pop();
	gs_projection_pop();
	gs_viewport_pop();
	//
	if (tex) {
		gs_texrender_end(tex);
	}
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




