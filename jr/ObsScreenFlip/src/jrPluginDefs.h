#pragma once

//---------------------------------------------------------------------------
// settings
#define Setting_Enabled						"enabled"
#define Setting_Enabled_Text				obs_module_text("Enable flip screen functionality")
#define Setting_Enabled_Def					true
#define Setting_OnlyDuringStreamRec			"onlyStreamRec"
#define Setting_OnlyDuringStreamRec_Text	obs_module_text("Engage only while streaming or recording")
#define Setting_OnlyDuringStreamRec_Def		false
#define Setting_SceneFilter					"sceneFilter"
#define Setting_SceneFilter_Text			obs_module_text("Scene filter criteria (sceneName | horizontalSplitPercentage%)")
#define Setting_SceneFilter_Def				"MainScene,50%\nSecondaryScene,30%"
//---------------------------------------------------------------------------
