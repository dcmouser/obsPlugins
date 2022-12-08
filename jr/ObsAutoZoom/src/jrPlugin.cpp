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
bool OnPropertyChangeCallback(obs_properties_t *props, obs_property_t *p, obs_data_t *settings) {
	UNUSED_PARAMETER(p);

	int keyColorMode = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_keyMode), (const char**)SETTING_zcKeyMode_choices, 0);
	obs_property_set_visible(obs_properties_get(props, "groupColorKeyingChroma"), (keyColorMode == 0));
	obs_property_set_visible(obs_properties_get(props, "groupColorKeyingHsv"), (keyColorMode != 0));	

	int markerMultiColorMode = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_markerMultiColorMode), (const char**)SETTING_markerMultiColorMode_choices, 0);
	bool showColor1 = (markerMultiColorMode != 1);
	bool showColor2 = (markerMultiColorMode != 0);
	bool showColorDual = (markerMultiColorMode > 1);

	obs_property_set_visible(obs_properties_get(props, SETTING_CHROMA_COLOR_TYPE1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_CHROMA_COLOR1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_SIMILARITY1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_SMOOTHNESS1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_HSV_COLOR_TYPE1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_HSV_COLOR1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_hueThreshold1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_saturationThreshold1), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_valueThreshold1), showColor1);

	obs_property_set_visible(obs_properties_get(props, SETTING_CHROMA_COLOR_TYPE2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_CHROMA_COLOR2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_SIMILARITY2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_SMOOTHNESS2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_HSV_COLOR_TYPE2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_HSV_COLOR2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_hueThreshold2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_saturationThreshold2), showColor2);
	obs_property_set_visible(obs_properties_get(props, SETTING_valueThreshold2), showColor2);

	//obs_property_set_visible(obs_properties_get(props, SETTING_DualColorGapFill), showColorDual);

	const char *type = obs_data_get_string(settings, SETTING_CHROMA_COLOR_TYPE1);
	bool custom = showColor1 && strcmp(type, "custom") == 0;
	obs_property_set_visible(obs_properties_get(props, SETTING_CHROMA_COLOR1), custom);
	//
	type = obs_data_get_string(settings, SETTING_CHROMA_COLOR_TYPE2);
	custom = showColor2 && strcmp(type, "custom") == 0;
	obs_property_set_visible(obs_properties_get(props, SETTING_CHROMA_COLOR2), custom);

	type = obs_data_get_string(settings, SETTING_HSV_COLOR_TYPE1);
	custom = showColor1 && strcmp(type, "custom") == 0;
	obs_property_set_visible(obs_properties_get(props, SETTING_HSV_COLOR1), custom);
	//
	type = obs_data_get_string(settings, SETTING_HSV_COLOR_TYPE2);
	custom = showColor2 && strcmp(type, "custom") == 0;
	obs_property_set_visible(obs_properties_get(props, SETTING_HSV_COLOR2), custom);

	//
	obs_property_set_visible(obs_properties_get(props, SETTING_dilateGreen), showColor1);
	obs_property_set_visible(obs_properties_get(props, SETTING_dilateRed), showColor2);

	int markerlessMode = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_markerlessMode), (const char**)SETTING_zcMarkerlessMode_choices, 0);
	obs_property_set_visible(obs_properties_get(props, "groupMarkerlessManualZoom"), (markerlessMode == 0));
	obs_property_set_visible(obs_properties_get(props, "groupMarkerlessPresets"), (markerlessMode == 1));	

	return true;
}


bool OnPropertyNSrcModifiedCallback(obs_properties_t *props, obs_property_t *property, obs_data_t *settings) {
	// sets the desired number of sources enabled in the options for user to choose
	int n_src = (int)obs_data_get_int(settings, SETTING_srcN);
	for (int i = 0; i < DefMaxSources; i++) {
		char name[16];
		snprintf(name, sizeof(name), SETTING_sourceNameWithArg, i);
		obs_property_t *p = obs_properties_get(props, name);
		obs_property_set_enabled(p, i < n_src);
		obs_property_set_visible(p, i < n_src);
	}
	for (int i = 0; i < DefMaxSources; i++) {
		char name[16];
		snprintf(name, sizeof(name), SETTING_sourceZoomScaleWithArg, i);
		obs_property_t *p = obs_properties_get(props, name);
		obs_property_set_enabled(p, i < n_src);
		obs_property_set_visible(p, i < n_src);
	}

	return true;
}



void onHotkeyCallback(void *data, obs_hotkey_id id, obs_hotkey_t *key, bool pressed) {
	if (!pressed)
		return;
	JrPlugin *plugin = (JrPlugin*) data;

	plugin->handleHotkeyPress(id, key);
}




bool OnPropertyButtonClickTrackOnAllSources(obs_properties_t *props, obs_property_t *property, void *data) {
	JrPlugin *plugin = (JrPlugin*) data;
	plugin->forceAllAnalyzeMarkersOnNextRender = true;
	return true;
}


bool OnPropertyButtonClickCalibrateMarkerSizesCallback(obs_properties_t *props, obs_property_t *property, void *data) {
	JrPlugin *plugin = (JrPlugin*) data;
	plugin->calibrateMarkerSizes();
	return true;
}

bool OnPropertyButtonClickCalibrateChromaCallback(obs_properties_t *props, obs_property_t *property, void *data) {
	JrPlugin *plugin = (JrPlugin*) data;
	plugin->calibrateMarkerChroma();
	return true;
}


bool OnPropertyButtonClickCalibrateZoomScaleCallback(obs_properties_t *props, obs_property_t *property, void *data) {
	JrPlugin *plugin = (JrPlugin*) data;
	plugin->calibrateSourceZoomScales();
	return true;
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
	JrPlugin::doGetPropertyDefauls(settings);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void plugin_update(void *data, obs_data_t *settings) {
	// this triggers a lot -- every time user changes any setting in options but ALSO whenever we manually set a setting, which is problematic
	JrPlugin *plugin = (JrPlugin*) data;
	//mydebug("Triggering plugin_update().");

	// reload any changed settings vals
	plugin->updateSettingsOnChange(settings);

	// ATTN: we could maybe call HERE the call recheckSizeAndAdjustIfNeeded RATHER than in tick()
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
	//mydebug("Triggering plugin_save().");
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void plugin_destroy(void *data)
{
	JrPlugin *plugin = (JrPlugin*) data;

	WaitForSingleObject(plugin->mutex, INFINITE);

	obs_enter_graphics();
		plugin->freeBeforeReallocateEffects();
	obs_leave_graphics();

	ReleaseMutex(plugin->mutex);
	CloseHandle(plugin->mutex);

	plugin->stracker.freeForFinalExit();

	// let go of ourselves? this is weird but i guess this is how we allocate construct ourselves
	bfree(data);
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void *plugin_create(obs_data_t *settings, obs_source_t *context)
{
	// CREATE (allocate space for) the new plugin and point to it; this is our plugin class instance that will be used throughout via plugin pointer
	JrPlugin *plugin = (JrPlugin *) bzalloc(sizeof(JrPlugin));

	// store pointer to obs context
	plugin->context = context;

	// remember our local plugin type
	const char* pluginIdStr = obs_source_get_id(context);
	//
	enum obs_source_type pluginObsType = obs_source_get_type(context);
	if (pluginObsType == OBS_SOURCE_TYPE_INPUT) {
		plugin->pluginType = EnumJrPluginTypeSource;
	} else if (pluginObsType == OBS_SOURCE_TYPE_FILTER) {
		plugin->pluginType = EnumJrPluginTypeFilter;
	} else {
		plugin->pluginType = EnumJrPluginTypeUnknown;
	}
	//mydebug("PLUGIN ID: %s typed as %d / %d.", pluginIdStr, (int)pluginObsType, (int)plugin->pluginType);

	// init stuff inside graphics
	obs_enter_graphics();
	bool success = plugin->initFilterInGraphicsContext();
	obs_leave_graphics();

	if (!success) {
		obserror("Aborting load of plugin due to failure to initialize (find effeects, etc.).");
		plugin_destroy(plugin);
		return NULL;
	}

	// one time startup init outside graphics context
	plugin->initFilterOutsideGraphicsContext();

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
	//mydebug("plugin_tick starts.");
	plugin->doTick();
	//mydebug("plugin_tick ends.");
}



void plugin_render(void* data, gs_effect_t* obsoleteFilterEffect) {
	UNUSED_PARAMETER(obsoleteFilterEffect);
	JrPlugin* plugin = (JrPlugin*) data;
	//mydebug("plugin_render starts.");
	plugin->doRender();
	//mydebug("plugin_render ends.");
}
//---------------------------------------------------------------------------














//---------------------------------------------------------------------------
void plugin_enum_sources(void *data, obs_source_enum_proc_t enum_callback, void *param) {

	if (DefDebugDontEnumerateSourceOnRequest) {
		return;
	}


	JrPlugin* plugin = (JrPlugin*) data;

	if (plugin->in_enumSources)
		return;
	plugin->in_enumSources = true;
	SourceTracker* strackerp = plugin->getSourceTrackerp();
	TrackedSource* tsp;
	int numSources = strackerp->getSourceCount();

	for (int i = 0; i < numSources; i++) {
		tsp = strackerp->getTrackedSourceByIndex(i);
		obs_source_t *src = tsp->borrowFullSourceFromWeakSource();
		if (!src)
			continue;
		enum_callback(plugin->context, src, param);
		tsp->releaseBorrowedFullSource(src); src = NULL;
	}

	plugin->in_enumSources = false;
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
	// ATTN: we are using the currently viewed source to return info about color space -- but this probably isnt right -- we probably should just return OUR forced color space

	JrPlugin *plugin = (JrPlugin*) data;

	const enum gs_color_space potential_spaces[] = {
		GS_CS_SRGB,
		GS_CS_SRGB_16F,
		GS_CS_709_EXTENDED,
	};

	// get current source (be sure to free it before we leave)
	TrackedSource* tsourcep = plugin->stracker.getCurrentSourceViewing();
	if (!tsourcep) {
		//mydebug("ERROR: NULL tsourcep in plugin_get_color_space.");
		return GS_CS_SRGB;
	}
	//
	obs_source_t* source = tsourcep->borrowFullSourceFromWeakSource();
	if (!source) {
		//mydebug("ERROR: NULL source in plugin_get_color_space.");
		return GS_CS_SRGB;
	}
	const enum gs_color_space source_space = obs_source_get_color_space(source,	OBS_COUNTOF(potential_spaces), potential_spaces);

	tsourcep->releaseBorrowedFullSource(source); source = NULL;
	return source_space;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
uint32_t plugin_width(void *data)
{
	JrPlugin *plugin = (JrPlugin*) data;
	if (!plugin->opt_resizeOutput || plugin->opt_filterBypass) {
		// when bypassing, pass through original dimension and dont obey any forced output size
		return (uint32_t)plugin->outputWidthAutomatic;
	}
	return (uint32_t)plugin->outputWidthPlugin;
}

uint32_t plugin_height(void *data)
{
	JrPlugin *plugin = (JrPlugin*) data;
	if (!plugin->opt_resizeOutput || plugin->opt_filterBypass) {
		// when bypassing, pass through original dimension and dont obey any forced output size
		return (uint32_t)plugin->outputHeightAutomatic;
	}
	return (uint32_t)plugin->outputHeightPlugin;
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
// internal signaling system by hijacking hotkey stuff
void pluginOnKeyClick(void* data, const struct obs_key_event* event, bool key_up) {
	JrPlugin *plugin = (JrPlugin*) data;

	// here is our test to see if the signal is really from us
	if (key_up != DefActionSignalStructPreset_keyUp || event->modifiers != DefActionSignalStructPreset_modifiers || event->native_modifiers != DefActionSignalStructPreset_native_modifiers || event->native_vkey != DefActionSignalStructPreset_native_vkey) {
		// not a special internal signal
		return;
	}

	// its a special internal signal, so process it
	uint32_t keyCode = event->native_scancode;
	if (key_up) {
		plugin->receiveVisibleActionSignal(keyCode);
	}
}
//---------------------------------------------------------------------------


















//---------------------------------------------------------------------------
void setObsPluginSourceInfoGeneric(obs_source_info *pluginInfo) {
	pluginInfo->version = 2;
	// note that the OBS_SOURCE_COMPOSITE flag below is supposed to be used by sources that composite multiple children, which we seem to do -- not sure its needed though
	// note that we need the OBS_SOURCE_INTERACTION flag so we can use our internal signaling system
	pluginInfo->output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_INTERACTION; /* | OBS_SOURCE_COMPOSITE */
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
	pluginInfo->icon_type = OBS_ICON_TYPE_CAMERA;
	// this is really used for our internal signaling system
	pluginInfo->key_click = pluginOnKeyClick;
}



// global plugin for obs
struct obs_source_info autoZoomSourcePlugin;
bool registerAutoZoomSourcePlugin() {
	// set params
	autoZoomSourcePlugin.id = DefMyPluginNameSource;
	autoZoomSourcePlugin.type = OBS_SOURCE_TYPE_INPUT;
	setObsPluginSourceInfoGeneric(&autoZoomSourcePlugin);
	// register it with obs
	obs_register_source(&autoZoomSourcePlugin);
	//
	return true;
}


struct obs_source_info autoZoomFilterPlugin;
bool registerAutoZoomFilterPlugin() {
	// set params
	autoZoomFilterPlugin.id = DefMyPluginNameFilter;
	autoZoomFilterPlugin.type = OBS_SOURCE_TYPE_FILTER;
	setObsPluginSourceInfoGeneric(&autoZoomFilterPlugin);
	// register it with obs
	obs_register_source(&autoZoomFilterPlugin);
	//
	return true;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
bool obs_module_load(void)
{
	registerAutoZoomSourcePlugin();
	registerAutoZoomFilterPlugin();
	return true;
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------
