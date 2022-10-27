#include "jrPlugin.h"

#include <../UI/obs-frontend-api/obs-frontend-api.h>










//---------------------------------------------------------------------------
void JrPlugin::doRender() {
	// for filter here is our source
	if (!getIsPluginTypeFilter()) {
		// we are a source just used for configuring callback, we don't actually render
		return;
	}

	obs_source_t* source = getThisPluginParentSource();

	bool flagRenderedFlip = renderSourceFlip(false, source);

	if (!flagRenderedFlip) {
		// just normal render
		if (sourceWidth == 0) {
			return;
		}
		jrRenderSourceOut(source, sourceWidth, sourceHeight, sourceWidth, sourceHeight, jrBlendClearOverwite);
	}
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
bool JrPlugin::renderSourceFlip(bool flagRenderCurrentScene, obs_source_t* source) {
	// initial checks
	if (source == NULL) {
		return false;
	}
	if (!isEnabled()) {
		return false;
	}

	// calc settings for scene, notably sceneSettingSplitPosition
	calcSettingsForScene(source);

	// proceed?
	if (sourceWidth == 0) {
		return false;
	}
	if (sceneSettingSplitPosition == 0) {
		return false;
	}

	// set effect parameter
	setupEffect(sceneSettingSplitPosition);

	if (flagRenderCurrentScene) {
		// special efficient operation for rendering main scene
		return renderFlipScreenMainDisplay();
	} else {
		return renderFlipScreen();
	}
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
bool JrPlugin::renderFlipScreen() {
	// draw source into texture first with default effect
	jrRenderSourceIntoTexture(renderSource, texrender, sourceWidth, sourceHeight, jrBlendClearOverwite);

	// now render effect from texture to output NULL=SCREEN or current target
	jrRenderEffectIntoTexture(NULL, effectMain, texrender, sourceWidth, sourceHeight, jrBlendClearOverwite, "Draw");

	return true;
}


bool JrPlugin::renderFlipScreenMainDisplay() {
	// this could be called in place of "obs_render_main_texture_src_color_only()" in "void OBSBasic::RenderMain(void *data, uint32_t cx, uint32_t cy)"
	obs_video_info ovi;
	obs_get_video_info(&ovi);
	int outWidth = ovi.base_width;
	int outHeight = ovi.base_height;

	// setup rendering to texture
	jrRenderTextureRenderStart(texrender, outWidth, outHeight, jrBlendClearOverwite);

	// ATTN: unclear if we need this
	gs_ortho(0.0f, (float)outWidth, 0.0f, (float)outHeight, -100.0f, 100.0f);

	// draw main display into texture
	obs_render_main_texture_src_color_only();

	// restore state and finalize texture
	JrRenderTextureRenderEnd(texrender);

	// now render effect from texture to output NULL=SCREEN or current target
	jrRenderEffectIntoTexture(NULL, effectMain, texrender, outWidth, outHeight, jrBlendClearOverwite, "Draw");

	return true;
}
//---------------------------------------------------------------------------


