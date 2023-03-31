project(ObsScreenFlip)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(ObsScreenFlip MODULE)

target_link_libraries(ObsScreenFlip PRIVATE OBS::libobs OBS::frontend-api Qt::Core Qt::Widgets Qt::Network)

set_target_properties(ObsScreenFlip PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "OBS Preview Window Streaming Mirror Screen Flip")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in ObsScreenFlip.rc)

target_sources(
	ObsScreenFlip
	PRIVATE
		src/ObsScreenFlip.cpp
		src/ObsScreenFlip.hpp
		src/plugininfo.hpp
		src/pluginRender.cpp
		src/pluginMainRenderHook.cpp
		src/obsSf_options.cpp
		src/obsSf_options.hpp
		../jrcommon/src/jrhelpers.cpp
		../jrcommon/src/jrhelpers.hpp
		../jrcommon/src/jrqthelpers.cpp
		../jrcommon/src/jrqthelpers.hpp
		../jrcommon/src/jrobshelpers.cpp
		../jrcommon/src/jrobshelpers.hpp
		../jrcommon/src/jrobsplugin.cpp
		../jrcommon/src/jrobsplugin.hpp
		../jrcommon/src/jrobsplugin_options.cpp
		../jrcommon/src/jrobsplugin_options.hpp

		)
		

target_link_libraries(ObsScreenFlip)

target_compile_definitions(
  ObsScreenFlip PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(ObsScreenFlip)
