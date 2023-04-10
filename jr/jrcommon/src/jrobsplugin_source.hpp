#pragma once

#include "jrobsplugin.hpp"

//---------------------------------------------------------------------------
enum EnumJrPluginType {EnumJrPluginTypeUnknown, EnumJrPluginTypeSource, EnumJrPluginTypeFilter};
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
class jrObsPluginSource: public jrObsPlugin {
public:
	// obs pointer should be initialized on creation of a source item
	obs_source_t *context = NULL;
public:
	EnumJrPluginType pluginType;
	int width = 640;
	int height = 480;
public:
	obs_source_t* renderSource = NULL;
	uint32_t sourceWidth, sourceHeight;
public:
	void updateSourceProperties();
	void setContext(obs_source_t* incontext) { context = incontext; }
	const char* getPluginIdCharp() { return obs_source_get_id(context); }
	EnumJrPluginType getPluginType() { return pluginType; }
	//
	obs_source_t* getThisPluginContextSource() { return context; }
	obs_source_t* getThisPluginParentSource() { return obs_filter_get_parent(context); }
	obs_source_t* getThisPluginTargetSource() { return obs_filter_get_target(context); }
	bool getIsPluginTypeFilter() { return (obs_source_get_type(context) == OBS_SOURCE_TYPE_FILTER); }
	void setJrPluginType(EnumJrPluginType in_pluginType) { pluginType = in_pluginType; };
public:
	void setObsPluginSourceSetCallbacks(obs_source_info* pluginInfo);
public:
	bool initFilterInGraphicsContext() { return true; };
	bool initFilterOutsideGraphicsContext() { return true; };
public:
	// class member functions called from obs global registered callbacks
	virtual bool gon_plugin_create(obs_data_t* settings, obs_source_t* context);
	virtual const char* gon_plugin_get_name() { return "jrObsPluginSourceBaseClassUnname"; };
	virtual int gon_plugin_width() { return width; };
	virtual int gon_plugin_height() { return height; };
	virtual void gon_plugin_tick(float t) { ; };
	virtual bool gon_plugin_render(gs_effect_t* obsoleteFilterEffect) { return true; };
	virtual void gon_plugin_show() { ; };
	virtual void gon_plugin_hide() { ; };
	//
	virtual obs_properties_t* gon_plugin_get_properties() {	return obs_properties_create();	};
	virtual void gon_plugin_update( obs_data_t* settings) { ; };
	//
	virtual void gon_plugin_load(obs_data_t* settings) { ; };
	virtual void gon_plugin_save(obs_data_t* settings) { ; };
	//
	virtual void gon_plugin_destroy() { ; };
	//
	virtual void gon_onHotkeyCallback(obs_hotkey_id id, obs_hotkey_t* key, bool pressed) { ; };
	//
	virtual void gon_plugin_enum_sources(obs_source_enum_proc_t enum_callback, void* param) { ; };
	virtual bool gon_plugin_audio_render(uint64_t* ts_out, struct obs_source_audio_mix* audio_output, uint32_t mixers, size_t channels, size_t sample_rate) {return false;};
	virtual enum gs_color_space gon_plugin_get_color_space(size_t count, const enum gs_color_space* preferred_spaces);
	//
	virtual void gon_pluginOnKeyClick(const struct obs_key_event* event, bool key_up) { ; };
	//
	static void gon_plugin_get_defaults(obs_data_t* settings) { ; };
	//
	virtual void gon_pluginModuleSingletonLoadDoRegistration() { ; };
public:
	void forceUpdatePluginSettingsOnOptionChange() { ; }
};
//---------------------------------------------------------------------------
