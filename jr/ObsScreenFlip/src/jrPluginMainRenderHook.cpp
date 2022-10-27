#include "jrplugin.h"
#include <obs.h>
#include <../UI/obs-frontend-api/obs-frontend-api.h>



//---------------------------------------------------------------------------
// put these two lines in your code:
typedef bool (*THookRenderMainCallbackFp) (void* data, bool flagRenderCurrentScene, obs_source_t* source, obs_display_t* display);
extern void obs_register_hook_rendermain_callback(THookRenderMainCallbackFp fp, void* data);
extern void obs_unregister_hook_rendermain_callback();
// they are defined now in obs.h and obs.c
// and then call obs_register_hook_rendermain_callback(functionName, pointerToYourClassInstance) to get a callback
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
static bool staticReceiveMainRenderCallbackFp(void* data, bool flagRenderCurrentScene, obs_source_t* source, obs_display_t* display) {
	//mydebug("IN JrPlugin::staticReceiveMainRenderCallbackFp.");
	JrPlugin* pluginp = (JrPlugin*)data;
	return pluginp->doReceiveMainRenderCallback(flagRenderCurrentScene, source, display);
}
//---------------------------------------------------------------------------















//---------------------------------------------------------------------------
void JrPlugin::registerMainRenderHook() {
	//mydebug("ATTN: registerMainRenderHook");
	obs_register_hook_rendermain_callback(staticReceiveMainRenderCallbackFp, this);
}

void JrPlugin::unRegisterMainRenderHook() {
	//mydebug("ATTN: unRegisterMainRenderHook");
	obs_unregister_hook_rendermain_callback();
}


bool JrPlugin::doReceiveMainRenderCallback(bool flagRenderCurrentScene, obs_source_t* source, obs_display_t* display) {
	if (!isEnabled()) {
		return false;
	}

	// on main display we are not passed an explicit source; we could also check flagRenderCurrentScene which should be true in this case
	// we will still have to get the name of the currently outputting source in order to see if we have an entry for it
	bool flagSourceNeedsRelease = false;
	if (source == NULL) {
		// just ask obs what the current scene source is -- we will stil render the more efficient chached main texture but this lets us get info like the scene name
		source = obs_frontend_get_current_scene();
		flagSourceNeedsRelease = true;
	}
	if (source == NULL) {
		return false;
	}

	bool bretv = renderSourceFlip(flagRenderCurrentScene, source);
	if (flagSourceNeedsRelease) {
		obs_source_release(source);
	}
	return bretv;
}
//---------------------------------------------------------------------------


