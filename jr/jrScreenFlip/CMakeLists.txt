project(jrScreenFlip)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrScreenFlip MODULE)

target_link_libraries(jrScreenFlip PRIVATE OBS::libobs OBS::frontend-api Qt::Core Qt::Widgets Qt::Network)

set_target_properties(jrScreenFlip PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "OBS Preview Window Streaming Mirror Screen Flip")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrScreenFlip.rc)

target_sources(
	jrScreenFlip
	PRIVATE
		src/jrScreenFlip.cpp
		src/jrScreenFlip.hpp
		src/plugininfo.hpp
		src/pluginRender.cpp
		src/pluginMainRenderHook.cpp
		src/jrScreenFlip_options.cpp
		src/jrScreenFlip.hpp
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
		

target_link_libraries(jrScreenFlip)

target_compile_definitions(
  jrScreenFlip PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrScreenFlip)
