//---------------------------------------------------------------------------
#include <cstdio>
//
#include "jrPlugin.h"
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// forward declarations that are extern "C"
#ifdef __cplusplus
extern "C" {
#endif
	bool OnPropertyKeyTypeChangeCallback(obs_properties_t* props, obs_property_t* p, obs_data_t* settings);
	bool OnPropertyNSrcModifiedCallback(obs_properties_t* props, obs_property_t* property, obs_data_t* settings);
	void onHotkeyCallback(void* data, obs_hotkey_id id, obs_hotkey_t* key, bool pressed);
	bool OnPropertyButtonClickTrackOnAllSources(obs_properties_t *props, obs_property_t *property, void *data);
	bool OnPropertyButtonClickCalibrateMarkerSizesCallback(obs_properties_t *props, obs_property_t *property, void *data);
	bool OnPropertyButtonClickCalibrateChromaCallback(obs_properties_t *props, obs_property_t *property, void *data);
	bool OnPropertyButtonClickCalibrateZoomScaleCallback(obs_properties_t *props, obs_property_t *property, void *data);
#ifdef __cplusplus
} // extern "C"
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// GLOBAL CHAR* ARRAYS USED IN SETTINGS PROPERTIES (would be better to have these as simple defines but awkward)
char* SETTING_zcMarkerPos_choices[] =			{ "outer", "innner", "center", NULL };
char* SETTING_zcAlignment_choices[] =			{ "topLeft", "topCenter", "topRight", "middleLeft", "center", "middleRight", "bottomLeft", "bottomCenter", "bottomRight", NULL };
char* SETTING_zcMode_choices[] =				{ "zoom and crop", "only crop", "only zoom", NULL };
char* SETTING_zcEasing_choices[] = 				{ "nstant","eased", NULL };
char* SETTING_fadeMode_choices[] = 				{ "none","normal", NULL };
char* SETTING_markerChromaMode_choices[] = 		{ "color 1", "color 2", "dual color", NULL};
char* SETTING_zcCropStyle_choices[] = 			{ "black bars", "blur", NULL};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
char* markerChromaModeRenderTechniques[] = {"DrawColor1", "DrawColor2", "DrawDualSep", NULL};
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
obs_properties_t* JrPlugin::doPluginAddProperties() {
	obs_properties_t *props = obs_properties_create();
	obs_properties_t* propgroup;
	obs_property_t *p;
	char name[16];
	char label[80];



	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupDebug", "Diagnostics/Debugging/Calibrating", OBS_GROUP_NORMAL, propgroup);
	//

	// debugging
	obs_properties_add_bool(propgroup, SETTING_debugRegions, TEXT_debugRegions);
	obs_properties_add_bool(propgroup, SETTING_debugChroma, TEXT_debugChroma);
	obs_properties_add_bool(propgroup, SETTING_debugAllUpdate, TEXT_debugAllUpdate);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupDisplay", "Display options", OBS_GROUP_NORMAL, propgroup);
	//
	obs_property_t* comboString = obs_properties_add_list(propgroup, SETTING_zcMode, TEXT_zcMode, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboString, (const char**)SETTING_zcMode_choices);
	comboString = obs_properties_add_list(propgroup, SETTING_zcCropStyle, TEXT_zcCropStyle, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboString, (const char**)SETTING_zcCropStyle_choices);
	//
	obs_properties_add_text(propgroup, SETTING_zcOutputSize, TEXT_zcOutputSize, OBS_TEXT_DEFAULT);
	//obs_properties_add_bool(propgroup, SETTING_resizeOutput, TEXT_resizeOutput);
	obs_properties_add_bool(propgroup, SETTING_zcPreserveAspectRatio, TEXT_zcPreserveAspectRatio);
	comboString = obs_properties_add_list(propgroup, SETTING_zcAlignment, TEXT_zcAlignment, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboString, (const char**)SETTING_zcAlignment_choices);
	obs_properties_add_int_slider(propgroup, SETTING_zcMaxZoom, TEXT_zcMaxZoom, 0, 1000, 1);
	comboString = obs_properties_add_list(propgroup, SETTING_zcMarkerPos, TEXT_zcMarkerPos, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboString, (const char**)SETTING_zcMarkerPos_choices);
	obs_properties_add_int_slider(propgroup, SETTING_zcBoxMargin, TEXT_zcBoxMargin, -100, 200, 1);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupBehavior", "Behavior options", OBS_GROUP_NORMAL, propgroup);
	//
	obs_properties_add_bool(propgroup, SETTING_ignoreMarkers, TEXT_ignoreMarkers);
	obs_properties_add_bool(propgroup, SETTING_enableAutoSourceHunting, TEXT_enableAutoSourceHunting);
	obs_properties_add_bool(propgroup, SETTING_enableAutoUpdate, TEXT_enableAutoUpdate);
	obs_properties_add_int_slider(propgroup, SETTING_updateRate, TEXT_updateRate, 1, 120, 1);
	//
	comboString = obs_properties_add_list(propgroup, SETTING_zcEasing, TEXT_zcEasing, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboString, (const char**)SETTING_zcEasing_choices);
	//
	comboString = obs_properties_add_list(propgroup, SETTING_fadeMode, TEXT_fadeMode, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboString, (const char**)SETTING_fadeMode_choices);
	obs_properties_add_int_slider(propgroup, SETTING_zcBoxMoveSpeed, TEXT_zcBoxMoveSpeed, 0, 100, 1);
	obs_properties_add_int_slider(propgroup, SETTING_zcReactionDistance, TEXT_zcReactionDistance, 1, 200, 1);
	obs_properties_add_int_slider(propgroup, SETTING_zcBoxMoveDelay, TEXT_zcBoxMoveDelay, 1, 50, 1);



	// chroma green screen stuff
	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupChroma", "ChromaKey options", OBS_GROUP_NORMAL, propgroup);
	//
	comboString = obs_properties_add_list(propgroup, SETTING_markerChromaMode, TEXT_markerChromaMode, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	jrAddPropertListChoices(comboString, (const char**)SETTING_markerChromaMode_choices);
	obs_property_set_modified_callback(comboString, OnPropertyKeyTypeChangeCallback);
	//
	p = obs_properties_add_list(propgroup, SETTING_COLOR_TYPE1, TEXT_COLOR_TYPE1, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, obs_module_text("Green"), "green");
	obs_property_list_add_string(p, obs_module_text("Blue"), "blue");
	obs_property_list_add_string(p, obs_module_text("Magenta"), "magenta");
	obs_property_list_add_string(p, obs_module_text("Custom"), "custom");
	obs_property_set_modified_callback(p, OnPropertyKeyTypeChangeCallback);
	obs_properties_add_color(propgroup, SETTING_KEY_COLOR1, TEXT_KEY_COLOR1);
	obs_properties_add_int_slider(propgroup, SETTING_SIMILARITY1, TEXT_SIMILARITY1, 1, 1000, 1);
	obs_properties_add_int_slider(propgroup, SETTING_SMOOTHNESS1, TEXT_SMOOTHNESS1, 1, 1000, 1);
	//
	p = obs_properties_add_list(propgroup, SETTING_COLOR_TYPE2, TEXT_COLOR_TYPE2, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, obs_module_text("Green"), "green");
	obs_property_list_add_string(p, obs_module_text("Blue"), "blue");
	obs_property_list_add_string(p, obs_module_text("Magenta"), "magenta");
	obs_property_list_add_string(p, obs_module_text("Custom"), "custom");
	obs_property_set_modified_callback(p, OnPropertyKeyTypeChangeCallback);
	obs_properties_add_color(propgroup, SETTING_KEY_COLOR2, TEXT_KEY_COLOR2);
	obs_properties_add_int_slider(propgroup, SETTING_SIMILARITY2, TEXT_SIMILARITY2, 1, 1000, 1);
	obs_properties_add_int_slider(propgroup, SETTING_SMOOTHNESS2, TEXT_SMOOTHNESS2, 1, 1000, 1);
	//
	obs_properties_add_int_slider(propgroup, SETTING_ChromaThreshold, TEXT_ChromaThreshold, 1, 1000, 1);
	obs_properties_add_int_slider(propgroup, SETTING_DualColorGapFill, TEXT_DualColorGapFill, 0, 5, 1);

	// calibrate


	// calibrate
	// calibration doesnt quite work right
	if (!DefDebugDisableChromaCalibrate) {
		obs_properties_add_button(propgroup, "CalibrateChroma", "THEN: Auto-calibrate marker chroma colors", OnPropertyButtonClickCalibrateChromaCallback);
	}

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupMarker", "Marker detection", OBS_GROUP_NORMAL, propgroup);
	//
	obs_properties_add_int_slider(propgroup, SETTING_rmStageSize, TEXT_rmStageSize,1,SETTING_Max_rmStageSize,1);
	obs_properties_add_int_slider(propgroup, SETTING_rmDensityMin, TEXT_rmDensityMin, 1, 100, 1);
	obs_properties_add_int_slider(propgroup, SETTING_rmAspectMin, TEXT_rmAspectMin, 0, 100, 1);
	obs_properties_add_int_slider(propgroup, SETTING_rmSizeMin, TEXT_rmSizeMin, 1, 200, 1);
	obs_properties_add_int_slider(propgroup, SETTING_rmSizeMax, TEXT_rmSizeMax, 1, 600, 1);
	//
	obs_properties_add_int_slider(propgroup, SETTING_rmMinColor2Percent, TEXT_rmMinColor2Percent, 0, 1000, 1);
	obs_properties_add_int_slider(propgroup, SETTING_rmMaxColor2Percent, TEXT_rmMaxColor2Percent, 0, 1000, 1);
	//
	obs_properties_add_int_slider(propgroup, SETTING_rmTooCloseDist, TEXT_rmTooCloseDist, 0, 400, 1);
	// calibrate
	obs_properties_add_button(propgroup, "UpdateAllSources", "FIRST: Update all source tracking", OnPropertyButtonClickTrackOnAllSources);
	obs_properties_add_button(propgroup, "CalibrateMarkers", "THEN: Auto-calibrate marker sizes, etc.", OnPropertyButtonClickCalibrateMarkerSizesCallback);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupSource", "Sources", OBS_GROUP_NORMAL, propgroup);
	//
	// add properites for sources with callback so we can enable and disable them based on how many user says they are using
	p = obs_properties_add_int(propgroup, SETTING_srcN, TEXT_srcN, 0, DefMaxSources-1, 1);
	obs_property_set_modified_callback(p, OnPropertyNSrcModifiedCallback);
	//
	for (int i = 0; i < DefMaxSources; i++) {
		snprintf(name, sizeof(name), SETTING_sourceNameWithArg, i);
		snprintf(label, sizeof(label), TEXT_sourceNameWithArg, i);
		addPropertyForASourceOption(propgroup, name, label);
	}

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupSourceZoomScale", "Source zoomscale modifiers (adjusts marker detection values above)", OBS_GROUP_NORMAL, propgroup);
	//
	for (int i = 0; i < DefMaxSources; i++) {
		snprintf(name, sizeof(name), SETTING_sourceZoomScaleWithArg, i);
		snprintf(label, sizeof(label), TEXT_sourceZoomScaleWithArg, i);
		obs_properties_add_int(propgroup, name, label, 1, 1000, 1);
	}
	// calibrate
	obs_properties_add_button(propgroup, "UpdateAllSources2", "1. FIRST: Update all source tracking", OnPropertyButtonClickTrackOnAllSources);
	obs_properties_add_button(propgroup, "CalibrateZoomScales", "THEN: Auto-calibrate other sources based on source 0", OnPropertyButtonClickCalibrateZoomScaleCallback);

	propgroup = obs_properties_create();
	obs_properties_add_group(props, "groupMarkerless", "Markerless options", OBS_GROUP_NORMAL, propgroup);
	//
	obs_properties_add_bool(propgroup, SETTING_enableMarkerlessCoordinates, TEXT_enableMarkerlessCoordinates);
	obs_properties_add_text(propgroup, SETTING_zcMarkerlessCycleList, TEXT_zcMarkerlessCycleList, OBS_TEXT_DEFAULT);
	obs_properties_add_int(propgroup, SETTING_markerlessCycleIndex, TEXT_markerlessCycleIndex, 0, 100, 1);
	//
	obs_properties_add_int(propgroup, SETTING_manualViewSourceIndex, TEXT_manualViewSourceIndex, 0, 10, 1);



	return props;
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
void JrPlugin::doGetPropertyDefauls(obs_data_t *settings) {
	//
	obs_data_set_default_string(settings, SETTING_markerChromaMode, SETTING_Def_markerChromaMode);
	obs_data_set_default_int(settings, SETTING_KEY_COLOR1, SETTING_Def_KEY_COLOR);
	obs_data_set_default_string(settings, SETTING_COLOR_TYPE1, SETTING_Def_COLOR_TYPE);
	obs_data_set_default_int(settings, SETTING_KEY_COLOR2, SETTING_Def_KEY_COLOR);
	obs_data_set_default_string(settings, SETTING_COLOR_TYPE2, SETTING_Def_COLOR_TYPE);
	obs_data_set_default_int(settings, SETTING_SIMILARITY1, SETTING_Def_SIMILARITY);
	obs_data_set_default_int(settings, SETTING_SMOOTHNESS1, SETTING_Def_SMOOTHNESS);
	obs_data_set_default_int(settings, SETTING_SIMILARITY2, SETTING_Def_SIMILARITY);
	obs_data_set_default_int(settings, SETTING_SMOOTHNESS2, SETTING_Def_SMOOTHNESS);
	obs_data_set_default_int(settings, SETTING_ChromaThreshold, SETTING_Def_ChromaThreshold);
	obs_data_set_default_int(settings, SETTING_DualColorGapFill, SETTING_Def_DualColorGapFill);
	//
	obs_data_set_default_bool(settings, SETTING_ignoreMarkers, false);

	//
	obs_data_set_default_bool(settings, SETTING_debugRegions, SETTING_Def_debugRegions);
	obs_data_set_default_bool(settings, SETTING_debugChroma, SETTING_Def_debugChroma);
	obs_data_set_default_bool(settings, SETTING_debugAllUpdate, SETTING_Def_debugAllUpdate);
	//
	obs_data_set_default_bool(settings, SETTING_enableAutoUpdate, SETTING_Def_enableAutoUpdate);
	obs_data_set_default_int(settings, SETTING_updateRate, SETTING_Def_updateRate);
	obs_data_set_default_bool(settings, SETTING_zcPreserveAspectRatio, SETTING_Def_zcPreserveAspectRatio);
	obs_data_set_default_string(settings, SETTING_zcMarkerPos, SETTING_Def_zcMarkerPos);
	obs_data_set_default_int(settings, SETTING_zcBoxMargin, SETTING_Def_zcBoxMargin);
	obs_data_set_default_int(settings, SETTING_zcBoxMoveSpeed, SETTING_Def_zcBoxMoveSpeed);
	obs_data_set_default_int(settings, SETTING_zcReactionDistance, SETTING_Def_zcReactionDistance);
	obs_data_set_default_int(settings, SETTING_zcBoxMoveDelay, SETTING_Def_zcBoxMoveDelay);
	//
	obs_data_set_default_string(settings, SETTING_zcEasing, SETTING_Def_zcEasing);
	obs_data_set_default_string(settings, SETTING_fadeMode, SETTING_Def_fadeMode);
	//
	obs_data_set_default_int(settings, SETTING_rmDensityMin, SETTING_Def_rmDensityMin);
	obs_data_set_default_int(settings, SETTING_rmAspectMin, SETTING_Def_rmAspectMin);
	obs_data_set_default_int(settings, SETTING_rmSizeMin, SETTING_Def_rmSizeMin);
	obs_data_set_default_int(settings, SETTING_rmSizeMax, SETTING_Def_rmSizeMax);
	//
	obs_data_set_default_int(settings, SETTING_rmMinColor2Percent, SETTING_Def_rmMinColor2Percent);
	obs_data_set_default_int(settings, SETTING_rmMaxColor2Percent, SETTING_Def_rmMaxColor2Percent);
	//
	obs_data_set_default_int(settings, SETTING_rmTooCloseDist, SETTING_Def_rmTooCloseDist);
	obs_data_set_default_int(settings, SETTING_rmStageSize, SETTING_Def_rmStageSize);
	//
	obs_data_set_default_string(settings, SETTING_zcAlignment, SETTING_Def_zcAlignment);
	//
	obs_data_set_default_string(settings, SETTING_zcMode, SETTING_Def_zcMode);
	obs_data_set_default_string(settings, SETTING_zcCropStyle, SETTING_Def_zcCropStyle);
	//
	obs_data_set_default_int(settings, SETTING_zcMaxZoom, SETTING_Def_zcMaxZoom);
	//
	obs_data_set_default_string(settings, SETTING_zcOutputSize, SETTING_Def_zcOutputSize);
	//obs_data_set_default_bool(settings, SETTING_resizeOutput, SETTING_Def_resizeOutput);
	//
	obs_data_set_default_int(settings, 	SETTING_manualViewSourceIndex, SETTING_DEF_manualViewSourceIndex);
	obs_data_set_default_bool(settings, SETTING_enableAutoSourceHunting, SETTING_Def_enableAutoSourceHunting);
	obs_data_set_default_bool(settings, SETTING_enableMarkerlessCoordinates, SETTING_Def_enableMarkerlessCoordinates);
	obs_data_set_default_string(settings, SETTING_zcMarkerlessCycleList, SETTING_Def_zcMarkerlessCycleList);
	obs_data_set_default_int(settings, SETTING_markerlessCycleIndex, SETTING_Def_markerlessCycleIndex);
	//
	obs_data_set_default_int(settings, SETTING_srcN, SETTING_Def_srcN);

	for (int i = 0; i < DefMaxSources; i++) {
		if (true) {
			char name[16];
			snprintf(name, sizeof(name), SETTING_sourceZoomScaleWithArg, i);
			obs_data_set_default_int(settings, name, 100);
		}
	}
}
//---------------------------------------------------------------------------





//---------------------------------------------------------------------------
void JrPlugin::reRegisterHotkeys() {
	// we attach the hotkeys EITHER to ourselves (if we are a source) or the sources if we are a filter

	obs_source_t* target = isPluginTypeFilter() ? getThisPluginFiltersAttachedSource() : getThisPluginSource();
	//
	if (hotkeyId_ToggleAutoUpdate==-1) hotkeyId_ToggleAutoUpdate = obs_hotkey_register_source(target, "autoZoom.hotkeyToggleAutoUpdate", "1 autoZoom - Toggle AutoUpdate", onHotkeyCallback, this);
	if (hotkeyId_toggleAutoSourceHunting==-1) hotkeyId_toggleAutoSourceHunting = obs_hotkey_register_source(target, "autoZoom.toggleAutoSourceHunting", "2 autoZoom - Toggle auto source switching", onHotkeyCallback, this);
	if (hotkeyId_OneShotZoomCrop==-1) hotkeyId_OneShotZoomCrop = obs_hotkey_register_source(target, "autoZoom.hotkeyOneShotZoomCrop", "3 autoZoom - One-shot ZoomCrop", onHotkeyCallback, this);
	if (hotkeyId_ToggleCropping==-1) hotkeyId_ToggleCropping = obs_hotkey_register_source(target, "autoZoom.hotkeyToggleCropping", "4 autoZoom - Toggle Cropping", onHotkeyCallback, this);

	//if (hotkeyId_ToggleBypass==-1) hotkeyId_ToggleBypass = obs_hotkey_register_source(target, "autoZoom.hotkeyToggleBypass", "5 autoZoom Toggle Bypass", onHotkeyCallback, this);
	if (hotkeyId_ToggleIgnoreMarkers==-1) hotkeyId_ToggleIgnoreMarkers = obs_hotkey_register_source(target, "autoZoom.toggleIgnoreMarkers", "5 autoZoom - Toggle ignore makers", onHotkeyCallback, this);

	// we dont use these any more, we use a unified hotkey for cycling source or markerless
	//if (hotkeyId_CycleSource==-1) hotkeyId_CycleSource = obs_hotkey_register_source(target, "autoZoom.hotkeyCycleSource", "6a autoZoom cycle source", onHotkeyCallback, this);
	//if (hotkeyId_CycleSourceBack==-1) hotkeyId_CycleSource = obs_hotkey_register_source(target, "autoZoom.hotkeyCycleSourceBack", "6b autoZoom - Cycle source back", onHotkeyCallback, this);
	//
	if (hotkeyId_CycleViewForward==-1) hotkeyId_CycleViewForward = obs_hotkey_register_source(target, "autoZoom.cycleViewForward", "6a autoZoom - Cycle view forward (markerless index or source if disabled)", onHotkeyCallback, this);
	if (hotkeyId_CycleViewBack==-1) hotkeyId_CycleViewBack = obs_hotkey_register_source(target, "autoZoom.cycleViewBack", "6b autoZoom - Cycle view backwards (markerless index or source if disabled)", onHotkeyCallback, this);
	//
	if (hotkeyId_toggleEnableMarkerlessUse==-1) hotkeyId_toggleEnableMarkerlessUse = obs_hotkey_register_source(target, "autoZoom.toggleEnableMarkerlessUse", "8 autoZoom - Toggle markerless use", onHotkeyCallback, this);
	if (hotkeyId_ToggleDebugDisplay==-1) hotkeyId_ToggleDebugDisplay = obs_hotkey_register_source(target, "autoZoom.hotkeyToggleDebugDisplay", "9 autoZoom - Toggle Debug Display", onHotkeyCallback, this);
	//
	if (hotkeyId_ToggleAutoUpdate == -1) {
		info("Failed to register one or more hotkeys.");
	}
	else {
		//info("Successfully registered hotkey for autoZoom source.");
	}
}
//---------------------------------------------------------------------------



































//---------------------------------------------------------------------------
void JrPlugin::updateSettingsOnChange(obs_data_t *settings) {
	//mydebug("In updateSettingsOnChange.");

	opt_markerChromaMode = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_markerChromaMode), (const char**)SETTING_markerChromaMode_choices, 0);
	opt_key_color1 = (uint32_t)obs_data_get_int(settings, SETTING_KEY_COLOR1);
	int64_t similarity1 = obs_data_get_int(settings, SETTING_SIMILARITY1);
	int64_t smoothness1 = obs_data_get_int(settings, SETTING_SMOOTHNESS1);
	opt_key_color2 = (uint32_t)obs_data_get_int(settings, SETTING_KEY_COLOR2);
	int64_t similarity2 = obs_data_get_int(settings, SETTING_SIMILARITY2);
	int64_t smoothness2 = obs_data_get_int(settings, SETTING_SMOOTHNESS2);

	const char *key_type1 = obs_data_get_string(settings, SETTING_COLOR_TYPE1);
	const char *key_type2 = obs_data_get_string(settings, SETTING_COLOR_TYPE2);
	//
	struct vec4 key_rgb;
	struct vec4 cb_v4;
	struct vec4 cr_v4;
	//
	uint32_t key_color1 = opt_key_color1;
	if (strcmp(key_type1, "green") == 0)
		key_color1 = 0x00FF00;
	else if (strcmp(key_type1, "blue") == 0)
		key_color1 = 0xFF9900;
	else if (strcmp(key_type1, "magenta") == 0)
		key_color1 = 0xFF00FF;

	uint32_t key_color2 = opt_key_color2;
	if (strcmp(key_type2, "green") == 0)
		key_color2 = 0x00FF00;
	else if (strcmp(key_type2, "blue") == 0)
		key_color2 = 0xFF9900;
	else if (strcmp(key_type2, "magenta") == 0)
		key_color2 = 0xFF00FF;
	//
	// chroma stuff
	if (true) {
		vec4_from_rgba(&key_rgb, key_color1 | 0xFF000000);
		static const float cb_vec[] = { -0.100644f, -0.338572f, 0.439216f, 0.501961f };
		static const float cr_vec[] = { 0.439216f, -0.398942f, -0.040274f, 0.501961f };
		memcpy(&cb_v4, cb_vec, sizeof(cb_v4));
		memcpy(&cr_v4, cr_vec, sizeof(cr_v4));
		opt_chroma1.x = vec4_dot(&key_rgb, &cb_v4);
		opt_chroma1.y = vec4_dot(&key_rgb, &cr_v4);
	}
	opt_similarity1 = (float)similarity1 / 1000.0f;
	opt_smoothness1 = (float)smoothness1 / 1000.0f;
	//
	if (true) {
		vec4_from_rgba(&key_rgb, key_color2 | 0xFF000000);
		static const float cb_vec[] = { -0.100644f, -0.338572f, 0.439216f, 0.501961f };
		static const float cr_vec[] = { 0.439216f, -0.398942f, -0.040274f, 0.501961f };
		memcpy(&cb_v4, cb_vec, sizeof(cb_v4));
		memcpy(&cr_v4, cr_vec, sizeof(cr_v4));
		opt_chroma2.x = vec4_dot(&key_rgb, &cb_v4);
		opt_chroma2.y = vec4_dot(&key_rgb, &cr_v4);
	}
	opt_similarity2 = (float)similarity2 / 1000.0f;
	opt_smoothness2 = (float)smoothness2 / 1000.0f;


	opt_chromaThreshold = (float)obs_data_get_int(settings, SETTING_ChromaThreshold) / 1000.0f;
	opt_chromaDualColorGapFill = (int)obs_data_get_int(settings, SETTING_DualColorGapFill);

	// debugging
	opt_debugRegions = obs_data_get_bool(settings, SETTING_debugRegions);
	opt_debugChroma = obs_data_get_bool(settings, SETTING_debugChroma);
	opt_debugAllUpdate = obs_data_get_bool(settings, SETTING_debugAllUpdate);
	//
	opt_ignoreMarkers = obs_data_get_bool(settings, SETTING_ignoreMarkers);
	opt_enableAutoUpdate = obs_data_get_bool(settings, SETTING_enableAutoUpdate);
	opt_updateRate = (int)obs_data_get_int(settings, SETTING_updateRate);
	//
	opt_rmTDensityMin = (float)obs_data_get_int(settings, SETTING_rmDensityMin) / 100.0f;
	opt_rmTAspectMin = (float)obs_data_get_int(settings, SETTING_rmAspectMin) / 100.0f;
	opt_rmSizeMin = (int)obs_data_get_int(settings, SETTING_rmSizeMin);
	opt_rmSizeMax = (int)obs_data_get_int(settings, SETTING_rmSizeMax);
	//
	opt_rmMinColor2Percent = (float)obs_data_get_int(settings, SETTING_rmMinColor2Percent) /1000.0f;
	opt_rmMaxColor2Percent = (float)obs_data_get_int(settings, SETTING_rmMaxColor2Percent) /1000.0f;
	//
	opt_rmTooCloseDist = (int)obs_data_get_int(settings, SETTING_rmTooCloseDist);
	//
	opt_rmStageSize = (int)obs_data_get_int(settings, SETTING_rmStageSize);
	//
	opt_zcPreserveAspectRatio = obs_data_get_bool(settings, SETTING_zcPreserveAspectRatio);
	//
	opt_zcMarkerPos = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_zcMarkerPos), (const char**)SETTING_zcMarkerPos_choices,0);
	opt_zcBoxMargin = (int)obs_data_get_int(settings, SETTING_zcBoxMargin);
	opt_zcBoxMoveSpeed = (int)obs_data_get_int(settings, SETTING_zcBoxMoveSpeed);
	opt_zcReactionDistance = (int)obs_data_get_int(settings, SETTING_zcReactionDistance);
	opt_zcBoxMoveDelay = (int)obs_data_get_int(settings, SETTING_zcBoxMoveDelay);
	opt_zcEasing = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_zcEasing), (const char**)SETTING_zcEasing_choices,0);
	opt_fadeMode = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_fadeMode), (const char**)SETTING_fadeMode_choices,0);
	//
	opt_zcAlign = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_zcAlignment), (const char**)SETTING_zcAlignment_choices,0);
	//
	opt_zcMode = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_zcMode), (const char**)SETTING_zcMode_choices,0);
	opt_zcCropStyle = jrPropertListChoiceFind(obs_data_get_string(settings, SETTING_zcCropStyle), (const char**)SETTING_zcCropStyle_choices,0);
	//
	opt_zcMaxZoom = (float)obs_data_get_int(settings, SETTING_zcMaxZoom) / 33.3f;
	//
	strncpy(opt_OutputSizeBuf, obs_data_get_string(settings, SETTING_zcOutputSize), 79);
	
	// markerless stuff
	opt_enableMarkerlessUse = obs_data_get_bool(settings, SETTING_enableMarkerlessCoordinates);

	opt_manualViewSourceIndex = (int)obs_data_get_int(settings, SETTING_manualViewSourceIndex);
	opt_enableAutoSourceHunting = obs_data_get_bool(settings, SETTING_enableAutoSourceHunting);
	//
	strncpy(opt_markerlessCycleListBuf, obs_data_get_string(settings, SETTING_zcMarkerlessCycleList), DefMarkerlessCycleListBufMaxSize);
	opt_markerlessCycleIndex = (int)obs_data_get_int(settings, SETTING_markerlessCycleIndex);

	// sources
	// here we parse the names of the sources the user has specified in the options, and record the names of these source; we will retrieve the source details and pointers later
	SourceTracker* strackerp = getSourceTrackerp();
	int userIndicatedSourceCount = (int)obs_data_get_int(settings, SETTING_srcN);
	if (userIndicatedSourceCount < 0)
		userIndicatedSourceCount = 0;
	else if (userIndicatedSourceCount > DefMaxSources-2) {
		userIndicatedSourceCount = DefMaxSources - 2;
	}

	// store sources by name
	//
	// new code to support use of this plugin as a filter.. note that if we are in filter mode, the first source is treated specially -- it will be reserved for use by the filter source
	int startingSourceIndex=0;
	if (isPluginTypeFilter()) {
		startingSourceIndex = 1;
		//mydebug("Setting up for FILTER OBSAUTOZOOM!!!!!!!!!!!!!!!");
	}

	//
	strackerp->reviseSourceCount(userIndicatedSourceCount+startingSourceIndex);
	for (int i = 0; i < userIndicatedSourceCount; i++) {
		char name[16];
		snprintf(name, sizeof(name), SETTING_sourceNameWithArg, i);
		strackerp->updateSourceIndexByName(i+startingSourceIndex, obs_data_get_string(settings, name));
		snprintf(name, sizeof(name), SETTING_sourceZoomScaleWithArg, i);
		strackerp->setSourceMarkerZoomScale(i, (float)obs_data_get_int(settings, name) / 100.0f);
	}
	//
	// initialize special tracked source 0 for filter
	if (isPluginTypeFilter()) {
		strackerp->setExternallyManagedTrackedSource(0,NULL);
	}

	// force this option for now, no need to let user adjust it
	opt_fadeDuration = DefFadeDuration;

	// cropzoom drawing technique
	if (opt_zcCropStyle == Def_zcCropStyles_blackBars) {
		strcpy(cropZoomTechnique, "Draw");
	} else if (opt_zcCropStyle == Def_zcCropStyle_blur) {
		//strcpy(cropZoomTechnique, "DrawBlur");
		// this does not do any bluring, but it leaves the cropped out regions visible (as in zoom-only mode), this is so we can blur AFTER using more specialized effct
		strcpy(cropZoomTechnique, "DrawSoftCrop");
	} else {
		strcpy(cropZoomTechnique, "Draw");
	}

	// fixed values for now
	opt_resizeOutput = true;
	ep_optBlurExteriorDullness = 0.5f;
	opt_filterBypass = false;
	ep_optBlurPasses = 6;
	ep_optBlurSizeReduction = 12;


	// and now make changes based on options changing
	forceUpdatePluginSettingsOnOptionChange();
}
//---------------------------------------------------------------------------







//---------------------------------------------------------------------------
void JrPlugin::addPropertyForASourceOption(obs_properties_t *pp, const char *name, const char *desc) {
	// add a propery list dropdown to let user choose a source by name
	obs_property_t *p;
	p = obs_properties_add_list(pp, name, desc, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	property_list_add_sources(p, getThisPluginSource());
}
//---------------------------------------------------------------------------










//---------------------------------------------------------------------------
void JrPlugin::saveVolatileSettings() {
	// called when we change a setting in runtime object that we want to push into the stored configured object in obs
	// these are mostly things that can be toggled with hotkeys
	// ATTN:TODO - decide whether you really DO want to push these to be remembered or reset them?
	// see https://obsproject.com/docs/reference-sources.html

	// borrow settings
	obs_data_t* settings = obs_source_get_settings(getThisPluginSource());

	// push values
	obs_data_set_bool(settings, SETTING_enableAutoUpdate, opt_enableAutoUpdate);
	obs_data_set_bool(settings, SETTING_debugRegions, opt_debugRegions);
	obs_data_set_bool(settings, SETTING_ignoreMarkers, opt_ignoreMarkers);
	//
	obs_data_set_bool(settings, SETTING_enableMarkerlessCoordinates, opt_enableMarkerlessUse);
	obs_data_set_bool(settings, SETTING_enableAutoSourceHunting, opt_enableAutoSourceHunting);
	obs_data_set_int(settings, SETTING_markerlessCycleIndex, opt_markerlessCycleIndex);
	obs_data_set_string(settings, SETTING_zcMode, jrStringFromListChoice(opt_zcMode,(const char**)SETTING_zcMode_choices));
	obs_data_set_string(settings, SETTING_zcCropStyle, jrStringFromListChoice(opt_zcMode,(const char**)SETTING_zcCropStyle_choices));

	// make sure this one is up to date
	opt_manualViewSourceIndex = stracker.getViewSourceIndex();
	obs_data_set_int(settings, SETTING_manualViewSourceIndex, opt_manualViewSourceIndex);
	

	// now push the saved properties
	obs_properties_t* props = obs_source_properties(getThisPluginSource());
	obs_properties_apply_settings(props, settings);

	// this would force trigger update call to update() function, which we do NOT want
	//obs_source_update(getThisPluginSource(), settings);

	// release settings
	obs_data_release(settings);
}



void JrPlugin::saveVolatileMarkerZoomScaleSettings(bool isCustomColor) {
	// called when we change a setting in runtime object that we want to push into the stored configured object in obs
	// these are mostly things that can be toggled with hotkeys
	// ATTN:TODO - decide whether you really DO want to push these to be remembered or reset them?
	// see https://obsproject.com/docs/reference-sources.html

	// borrow settings
	obs_data_t* settings = obs_source_get_settings(getThisPluginSource());

	// push values
	obs_data_set_int(settings, SETTING_rmDensityMin, (int)(opt_rmTDensityMin * 100));
	obs_data_set_int(settings, SETTING_rmAspectMin, (int)(opt_rmTAspectMin * 100));
	obs_data_set_int(settings, SETTING_rmSizeMin, (int)opt_rmSizeMin);
	obs_data_set_int(settings, SETTING_rmSizeMax, (int)opt_rmSizeMax);
	obs_data_set_int(settings, SETTING_rmTooCloseDist, (int)opt_rmTooCloseDist);

	if (isCustomColor) {
		obs_data_set_string(settings, SETTING_COLOR_TYPE1, "custom");
		obs_data_set_int(settings, SETTING_KEY_COLOR1, opt_key_color1);
		obs_data_set_string(settings, SETTING_COLOR_TYPE2, "custom");
		obs_data_set_int(settings, SETTING_KEY_COLOR2, opt_key_color2);	
	}
	
	//
	for (int i = 0; i < stracker.getSourceCount(); i++) {
		char name[16];
		snprintf(name, sizeof(name), SETTING_sourceZoomScaleWithArg, i);
		obs_data_set_int(settings, name, (int)(stracker.sourceZoomScale[i] * 100));
	}

	// now push the saved properties
	obs_properties_t* props = obs_source_properties(getThisPluginSource());
	obs_properties_apply_settings(props, settings);

	// release settings
	obs_data_release(settings);
}
//---------------------------------------------------------------------------




