#include <obs-module.h>

#include "jrobsplugin_source.hpp"
#include "jrobshelpers.hpp"

//---------------------------------------------------------------------------
// forward declaration
extern "C" { void setPluginCallbacks(obs_source_info* pluginInfo); };
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrObsPluginSource::setObsPluginSourceSetCallbacks(obs_source_info* pluginInfo) {
	setPluginCallbacks(pluginInfo);
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void jrObsPluginSource::updateSourceProperties() {
	if (getIsPluginTypeFilter()) {
		//mydebug("In updateSourceProperties FILTER!!!! TYPE rendersource = %p", renderSource);
		renderSource = getThisPluginTargetSource();
		// attempt to properly get size of previous filter output
		sourceWidth = obs_source_get_base_width(renderSource);
		sourceHeight = obs_source_get_base_height(renderSource);
	}
	else {
		sourceWidth = width;
		sourceHeight = height;
		renderSource = getThisPluginTargetSource();
		return;

		renderSource = getThisPluginParentSource();
		//mydebug("In updateSourceProperties SOURCE TYPE rendersource = %p", renderSource);
		sourceWidth = obs_source_get_base_width(renderSource);
		sourceHeight = obs_source_get_base_height(renderSource);
		renderSource = getThisPluginTargetSource();
		//mydebug("In updateSourceProperties SOURCE TYPEB rendersource = %p", renderSource);
		// ATTN: I don't understand why this suddenly broke
	}
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool jrObsPluginSource::gon_plugin_create(obs_data_t* settings, obs_source_t* context) {
	// store pointer to obs context
	setContext(context);

	// remember our local plugin type
	const char* pluginIdStr = obs_source_get_id(context);
	//
	enum obs_source_type pluginObsType = obs_source_get_type(context);
	if (pluginObsType == OBS_SOURCE_TYPE_INPUT) {
		setJrPluginType(EnumJrPluginTypeSource);
	} else if (pluginObsType == OBS_SOURCE_TYPE_FILTER) {
		setJrPluginType(EnumJrPluginTypeFilter);
	} else {
		setJrPluginType(EnumJrPluginTypeUnknown);
	}
	//mydebug("PLUGIN ID: %s typed as %d / %d.", pluginIdStr, (int)pluginObsType, (int)plugin->pluginType);

	// init stuff inside graphics
	obs_enter_graphics();
	bool success = this->initFilterInGraphicsContext();
	obs_leave_graphics();

	if (!success) {
		obserror("Aborting load of plugin due to failure to initialize (find effeects, etc.).");
		return false;
	}

	// one time startup init outside graphics context
	this->initFilterOutsideGraphicsContext();

	/*
	// update? im not sure this is ready yet..
	if (true) {
		plugin_update(plugin, settings);
	}
	*/

	// force initial settings update
	gon_plugin_update(settings);

	return true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
enum gs_color_space jrObsPluginSource::gon_plugin_get_color_space(size_t count, const enum gs_color_space* preferred_spaces) {
	const enum gs_color_space potential_spaces[] = {
		GS_CS_SRGB,
		GS_CS_SRGB_16F,
		GS_CS_709_EXTENDED,
	};

	// ATTN: unfinished
	return GS_CS_SRGB;
};
//---------------------------------------------------------------------------
