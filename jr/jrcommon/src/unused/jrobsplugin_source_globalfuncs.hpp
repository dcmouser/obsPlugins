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
void* plugin_create(obs_data_t *settings, obs_source_t *context);
//
const char* plugin_get_name(void* data);
uint32_t plugin_width(void* data);
uint32_t plugin_height(void* data);
//
void plugin_tick(void* data, float t);
void plugin_render(void* data, gs_effect_t* obsoleteFilterEffect);
void plugin_show(void* data);
void plugin_hide(void* data);
//
obs_properties_t* plugin_get_properties(void* data);
void plugin_get_defaults(obs_data_t* settings);
void plugin_update(void* data, obs_data_t* settings);
void plugin_load(void* data, obs_data_t* settings);
void plugin_save(void* data, obs_data_t* settings);
void plugin_destroy(void* data);
//
void onHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed);
//
void plugin_enum_sources(void* data, obs_source_enum_proc_t enum_callback, void* param);
bool plugin_audio_render(void* data, uint64_t* ts_out, struct obs_source_audio_mix* audio_output, uint32_t mixers, size_t channels, size_t sample_rate);
enum gs_color_space plugin_get_color_space(void* data, size_t count, const enum gs_color_space* preferred_spaces);
//
//
void pluginOnKeyClick(void* data, const struct obs_key_event* event, bool key_up);
//---------------------------------------------------------------------------





















//---------------------------------------------------------------------------
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------
