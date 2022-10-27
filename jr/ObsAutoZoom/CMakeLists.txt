project(ObsAutoZoom)

add_library(ObsAutoZoom MODULE)

target_link_libraries(ObsAutoZoom PRIVATE OBS::libobs OBS::frontend-api)

set_target_properties(ObsAutoZoom PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "OBS Multi-camera chroma auto zoom and crop")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in ObsAutoZoom.rc)

target_sources(ObsAutoZoom PRIVATE ObsAutoZoom.rc src/source_list.cpp src/pluginCodeSetttings.cpp src/pluginCodeRender.cpp src/pluginCodeMisc.cpp src/pluginCodeMain.cpp src/pluginCodeHelper.cpp src/pluginCodeCalibrate.cpp src/obsHelpers.cpp src/jrTrackedSourceZoomBox.cpp src/jrTrackedSourceRender.cpp src/jrTrackedSourceRegionDetect.cpp src/jrTrackedSourceMisc.cpp src/jrTrackedSource.cpp src/jrSourceTracker.cpp src/jrRegionDetector.cpp src/jrPlugin.cpp src/jrMarkerlessManager.cpp src/jrfuncs.cpp)

target_link_libraries(ObsAutoZoom)

target_compile_definitions(
  ObsAutoZoom PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(ObsAutoZoom)
