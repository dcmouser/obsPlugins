// caller should first include plugininfo.hpp with macro definition for PLUGIN_CLASS
#include <obs.hpp>
#include <util/platform.h>
#include <obs-frontend-api.h>
#include <../obs-app.hpp>


//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
static PLUGIN_CLASS* moduleInstance = NULL;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void* plugin_create(obs_data_t* settings, obs_source_t* context)
	{
		// CREATE (allocate space for) the new plugin and point to it; this is our plugin class instance that will be used throughout via plugin pointer

		PLUGIN_CLASS* pluginp = new PLUGIN_CLASS();
		bool bretv = pluginp->gon_plugin_create(settings, context);
		if (!bretv) {
			delete pluginp;
			return NULL;
		}
		return pluginp;
	}


void plugin_destroy(void* data) {
	PLUGIN_CLASS* pluginp = (PLUGIN_CLASS*)data;
	pluginp->gon_plugin_destroy();
	// now delete -- without this we leak memory
	delete pluginp;
};
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
const char* plugin_get_name(void* data) { if (data == NULL) return moduleInstance->gon_plugin_get_name(); else return ((PLUGIN_CLASS*)data)->gon_plugin_get_name(); };
uint32_t plugin_width(void* data) { return ((PLUGIN_CLASS*)data)->gon_plugin_width(); };
uint32_t plugin_height(void* data) { return ((PLUGIN_CLASS*)data)->gon_plugin_height(); };
//
void plugin_tick(void* data, float t) { ((PLUGIN_CLASS*)data)->gon_plugin_tick(t); };
void plugin_render(void* data, gs_effect_t* obsoleteFilterEffect) { ((PLUGIN_CLASS*)data)->gon_plugin_render(obsoleteFilterEffect); };
void plugin_show(void* data) { ((PLUGIN_CLASS*)data)->gon_plugin_show(); };
void plugin_hide(void* data) { ((PLUGIN_CLASS*)data)->gon_plugin_hide(); };
//
obs_properties_t* plugin_get_properties(void* data) { return ((PLUGIN_CLASS*)data)->gon_plugin_get_properties(); };
void plugin_get_defaults(obs_data_t* settings) { PLUGIN_CLASS::gon_plugin_get_defaults(settings); };
void plugin_update(void* data, obs_data_t* settings) { ((PLUGIN_CLASS*)data)->gon_plugin_update(settings); };
//
void plugin_load(void* data, obs_data_t* settings) { ((PLUGIN_CLASS*)data)->gon_plugin_load(settings); };
void plugin_save(void* data, obs_data_t* settings) { ((PLUGIN_CLASS*)data)->gon_plugin_save(settings); };
//
void onHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed) { ((PLUGIN_CLASS*)data)->gon_onHotkeyCallback(id, key, pressed); };
//
void plugin_enum_sources(void* data, obs_source_enum_proc_t enum_callback, void* param) { ((PLUGIN_CLASS*)data)->gon_plugin_enum_sources(enum_callback, param); };
bool plugin_audio_render(void* data, uint64_t* ts_out, struct obs_source_audio_mix* audio_output, uint32_t mixers, size_t channels, size_t sample_rate) {return ((PLUGIN_CLASS*)data)->gon_plugin_audio_render(ts_out, audio_output, mixers, channels, sample_rate);};
enum gs_color_space plugin_get_color_space(void* data, size_t count, const enum gs_color_space* preferred_spaces) { return ((PLUGIN_CLASS*)data)->gon_plugin_get_color_space(count, preferred_spaces); };
//
void pluginOnKeyClick(void* data, const struct obs_key_event* event, bool key_up) { ((PLUGIN_CLASS*)data)->gon_pluginOnKeyClick(event, key_up); };
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// call this function from our object to set these callbacks
void setPluginCallbacks(obs_source_info* pluginInfo) {
	pluginInfo->get_name = plugin_get_name;
	pluginInfo->create = plugin_create;
	pluginInfo->destroy = plugin_destroy;
	pluginInfo->load = plugin_load;
	pluginInfo->save = plugin_save;
	pluginInfo->video_tick = plugin_tick;
	pluginInfo->video_render = plugin_render;
	pluginInfo->update = plugin_update;
	pluginInfo->get_properties = plugin_get_properties;
	pluginInfo->get_defaults = plugin_get_defaults;
	pluginInfo->video_get_color_space = plugin_get_color_space;
	pluginInfo->get_width = plugin_width;
	pluginInfo->get_height = plugin_height;
	pluginInfo->enum_active_sources = plugin_enum_sources;
	pluginInfo->audio_render = plugin_audio_render;
	// this is really used for our internal signaling system
	pluginInfo->key_click = pluginOnKeyClick;
	//
	pluginInfo->show = plugin_show;
	pluginInfo->hide = plugin_hide;
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
OBS_DECLARE_MODULE();
OBS_MODULE_AUTHOR(PLUGIN_AUTHOR);
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US");
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
static bool moduleInstanceIsRegisteredAndAutoDeletedByObs = false;

bool obs_module_load(void) {
	blog(LOG_INFO, "plugin loaded successfully (version %s)", PLUGIN_VERSION);
	moduleInstance = new PLUGIN_CLASS();
	// record that this object is our global singleton helper
	moduleInstance->setThisIsSingletonRep(true);
	// register sources etc.
	moduleInstance->gon_pluginModuleSingletonLoadDoRegistration();
	return true;
}

void obs_module_unload() {
	if (moduleInstance != NULL) {
		moduleInstance->onModuleUnload();
	}

	if (moduleInstanceIsRegisteredAndAutoDeletedByObs) {
		blog(LOG_INFO, "plugin managed by and should be auto deleted by OBS.");
		return;
	}
	blog(LOG_INFO, "plugin unloading");
	//
	if (moduleInstance != NULL) {
		delete moduleInstance;
		moduleInstance = NULL;
	}
	blog(LOG_INFO, "plugin unloaded");
}


void obs_module_post_load() {
	//blog(LOG_INFO, "plugin in onModulePostLoad");
	if (moduleInstance != NULL) {
		moduleInstance->onModulePostLoad();
	}
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------
