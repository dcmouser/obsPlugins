// base OBS plugin callbacks, etc.
// extern C wrapped code that can be called by OBS
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include <cstdio>
//
// obs helper
#include <util/dstr.h>
//
#include "jrPlugin.h"
#include "jrfuncs.h"
#include "jrPluginDefs.h"
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(DefMyPluginName, "en-US")
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
// CUSTOM REGISTERED OBS CALLBACKS
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
bool OnPropertyKeyTypeChangeCallback(obs_properties_t *props, obs_property_t *p, obs_data_t *settings) {
	UNUSED_PARAMETER(p);

	return true;
}


void onHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	JrPlugin *plugin = (JrPlugin*) data;

	plugin->handleHotkeyPress(id, key);
}
//---------------------------------------------------------------------------











//---------------------------------------------------------------------------
// PLUGIN REGISTERED AND INVOKED FUNCTIONS
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
obs_properties_t *plugin_get_properties(void* data)
{
	// get the properties for the plugin
	JrPlugin *plugin = (JrPlugin*) data;
	return plugin->doPluginAddProperties();
}

void plugin_get_defaults(obs_data_t *settings)
{
	// set default values of all options -- note this has to use a static function
	JrPlugin::doGetPropertyDefaults(settings);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void plugin_update(void *data, obs_data_t *settings) {
	// this triggers a lot -- every time user changes any setting in options but ALSO whenever we manually set a setting, which is problematic
	JrPlugin *plugin = (JrPlugin*) data;
	// reload any changed settings vals
	plugin->updateSettingsOnChange(settings);
}

void plugin_load(void *data, obs_data_t *settings) {
	// does this ONLY happen at startup? in log it seems like update() is called FIRST on startup and then comes load()
	JrPlugin* plugin = (JrPlugin*) data;
	//mydebug("Triggering plugin_load().");

	// register hotkeys
	plugin->reRegisterHotkeys();
}


void plugin_save(void *data, obs_data_t *settings) {
	// this triggers every time we CLOSE options dialog? (though not at startup), including calls during shutdown
	JrPlugin* plugin = (JrPlugin*) data;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void plugin_destroy(void *data)
{
	JrPlugin *plugin = (JrPlugin*) data;

	plugin->deInitialize();

	// let go of ourselves? this is weird but i guess this is how we allocate construct ourselves
	bfree(data);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void *plugin_create(obs_data_t *settings, obs_source_t *context)
{
	// CREATE (allocate space for) the new plugin and point to it; this is our plugin class instance that will be used throughout via plugin pointer
	JrPlugin *plugin = (JrPlugin *) bzalloc(sizeof(JrPlugin));
	bool success = true;

	// store pointer to obs context
	plugin->context = context;

	// remember our local plugin type
	const char* pluginIdStr = obs_source_get_id(context);

	// init
	success = plugin->init();

	if (!success) {
		obserror("Aborting load of plugin due to failure to initialize (find effeects, etc.).");
		plugin_destroy(plugin);
		return NULL;
	}

	// update? im not sure this is ready yet..
	if (true) {
		plugin_update(plugin, settings);
	}

	// return the plugin instance we have created
	return plugin;
}
//---------------------------------------------------------------------------




















//---------------------------------------------------------------------------
void plugin_tick(void *data, float t) {
	JrPlugin *plugin = (JrPlugin*) data;
	plugin->doTick();
}



void plugin_render(void* data, gs_effect_t* obsoleteFilterEffect) {
	UNUSED_PARAMETER(obsoleteFilterEffect);
	JrPlugin* plugin = (JrPlugin*) data;
	plugin->doRender();
}
//---------------------------------------------------------------------------









//---------------------------------------------------------------------------
bool plugin_audio_render(void* data, uint64_t* ts_out, struct obs_source_audio_mix* audio_output, uint32_t mixers, size_t channels, size_t sample_rate) {
	// dont want any audio
	// ATTN: does this do anything
	return false;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
enum gs_color_space plugin_get_color_space(void *data, size_t count,  const enum gs_color_space *preferred_spaces) {
	JrPlugin *plugin = (JrPlugin*) data;

	const enum gs_color_space potential_spaces[] = {
		GS_CS_SRGB,
		GS_CS_SRGB_16F,
		GS_CS_709_EXTENDED,
	};

	// ATTN: unfinished
	return GS_CS_SRGB;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
uint32_t plugin_width(void *data)
{
	JrPlugin *plugin = (JrPlugin*) data;
	// ATTN: unfinished
	return 1920;
}

uint32_t plugin_height(void *data)
{
	JrPlugin *plugin = (JrPlugin*) data;
	// ATTN: unfinished
	return 1080;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
// for registering the plugin with OBS

const char *plugin_get_name(void *data)
{
	// we may not be able to get info about the plugin data yet since it's not instantiated but we could return generic name without suffix saying source or filter
	// this should be ok since its just text name for nice display, though it would be nice for debugging to have the Source or Filter suffix at end of name
	//struct obs_source_info* si = (obs_source_info*)data;
	//return si->id;
	return obs_module_text(DefMyPluginName);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void setObsPluginSourceInfoGeneric(obs_source_info *pluginInfo) {
	pluginInfo->version = 2;
	// note that the OBS_SOURCE_COMPOSITE flag below is supposed to be used by sources that composite multiple children, which we seem to do -- not sure its needed though
	pluginInfo->output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB | OBS_SOURCE_CUSTOM_DRAW;
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
	pluginInfo->audio_render = plugin_audio_render;
	pluginInfo->icon_type = OBS_ICON_TYPE_CAMERA;
}



// global plugin for obs
struct obs_source_info screenFlipSourcePlugin;
bool registerScreenFlipSourcePlugin() {
	// set params
	screenFlipSourcePlugin.id = DefMyPluginNameSource;
	screenFlipSourcePlugin.type = OBS_SOURCE_TYPE_INPUT;
	setObsPluginSourceInfoGeneric(&screenFlipSourcePlugin);
	// register it with obs
	obs_register_source(&screenFlipSourcePlugin);
	//
	return true;
}


struct obs_source_info screenFlipFilterPlugin;
bool registerScreenFlipFilterPlugin() {
	// set params
	screenFlipFilterPlugin.id = DefMyPluginNameFilter;
	screenFlipFilterPlugin.type = OBS_SOURCE_TYPE_FILTER;
	setObsPluginSourceInfoGeneric(&screenFlipFilterPlugin);
	// register it with obs
	obs_register_source(&screenFlipFilterPlugin);
	//
	return true;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool obs_module_load(void)
{
	registerScreenFlipSourcePlugin();
	registerScreenFlipFilterPlugin();
	return true;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------
