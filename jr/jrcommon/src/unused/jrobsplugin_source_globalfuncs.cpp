#include "jrobsplugin_source_globalfuncs.hpp"

//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
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
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
const char* plugin_get_name(void* data) { return (PLUGIN_CLASS*)data->gon_plugin_get_name(); };
uint32_t plugin_width(void* data) { return (PLUGIN_CLASS*)data->gon_plugin_width(); };
uint32_t plugin_height(void* data) { return (PLUGIN_CLASS*)data->gon_plugin_height(); };
//
void plugin_tick(void* data, float t) { (PLUGIN_CLASS*)data->gon_plugin_tick(t); };
void plugin_render(void* data, gs_effect_t* obsoleteFilterEffect) { (PLUGIN_CLASS*)data->gon_plugin_render(); };
void plugin_show(void* data) { (PLUGIN_CLASS*)data->gon_plugin_show(); };
void plugin_hide(void* data) { (PLUGIN_CLASS*)data->gon_plugin_hide(); };
//
obs_properties_t* plugin_get_properties(void* data) { return (PLUGIN_CLASS*)data->gon_plugin_get_properties(); };
void plugin_get_defaults(obs_data_t* settings) { PLUGIN_CLASS::gon_plugin_get_defaults(settings); };
void plugin_update(void* data, obs_data_t* settings) { (PLUGIN_CLASS*)data->gon_plugin_update(settings); };
//
void plugin_load(void* data, obs_data_t* settings) { (PLUGIN_CLASS*)data->gon_plugin_load(settings); };
void plugin_save(void* data, obs_data_t* settings) { (PLUGIN_CLASS*)data->gon_plugin_save(settings); };
//
void plugin_destroy(void* data) { (PLUGIN_CLASS*)data->gon_plugin_destroy(); };
//
void onHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed) { (PLUGIN_CLASS*)data->gon_onHotkeyCallback(id, key, pressed); };
//
void plugin_enum_sources(void* data, obs_source_enum_proc_t enum_callback, void* param) { (PLUGIN_CLASS*)data->gon_plugin_enum_sources(enum_callback, param); };
bool plugin_audio_render(void* data, uint64_t* ts_out, struct obs_source_audio_mix* audio_output, uint32_t mixers, size_t channels, size_t sample_rate) {return (PLUGIN_CLASS*)data->gon_plugin_audio_render(ts_out, audio_output, mixers, channels, sample_rate);};
enum gs_color_space plugin_get_color_space(void* data, size_t count, const enum gs_color_space* preferred_spaces) { return (PLUGIN_CLASS*)data->gon_plugin_get_color_space(); };
//
void pluginOnKeyClick(void* data, const struct obs_key_event* event, bool key_up) { (PLUGIN_CLASS*)data->gon_pluginOnKeyClick(event, key_up); };
//---------------------------------------------------------------------------
















































	


//---------------------------------------------------------------------------
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------
