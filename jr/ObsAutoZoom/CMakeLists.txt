project(ObsAutoZoom)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)

add_library(ObsAutoZoom MODULE)

target_link_libraries(ObsAutoZoom PRIVATE OBS::libobs OBS::frontend-api Qt::Core Qt::Widgets Qt::Network)

set_target_properties(ObsAutoZoom PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "OBS Multi-camera chroma auto zoom and crop")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in ObsAutoZoom.rc)

target_sources(
	ObsAutoZoom
	PRIVATE
		../jrcommon/src/jrhelpers.cpp
		../jrcommon/src/jrhelpers.hpp
		../jrcommon/src/jrqthelpers.cpp
		../jrcommon/src/jrqthelpers.hpp
		../jrcommon/src/jrobshelpers.cpp
		../jrcommon/src/jrobshelpers.hpp
		
		src/plugininfo.h
		src/pluginCodeSetttings.cpp
		src/pluginCodeRender.cpp
		src/pluginCodeMisc.cpp
		src/pluginCodeMain.cpp
		src/pluginCodeHelper.cpp
		src/pluginCodeCalibrate.cpp
		src/jrTrackedSourceZoomBox.cpp
		src/jrTrackedSourceRender.cpp
		src/jrTrackedSourceRegionDetect.cpp
		src/jrTrackedSourceMisc.cpp
		src/jrTrackedSource.h
		src/jrTrackedSource.cpp
		src/jrSourceTracker.h
		src/jrSourceTracker.cpp
		src/jrRegionDetector.h
		src/jrRegionDetector.cpp
		src/jrPlugin.h
		src/jrPlugin.cpp
		src/jrPluginDefs.h
		src/jrMarkerlessManager.h
		src/jrMarkerlessManager.cpp
		src/pluginCodePerform.cpp
		src/ObsAutoZoom.h
		)


target_link_libraries(ObsAutoZoom)

target_compile_definitions(
  ObsAutoZoom PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(ObsAutoZoom)
