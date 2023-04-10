project(jrborder)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrborder MODULE)

target_link_libraries(jrborder PRIVATE OBS::libobs OBS::frontend-api Qt::Core Qt::Widgets Qt::Network)

set_target_properties(jrborder PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "OBS Filter adds borders to sources")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrborder.rc)

target_sources(
	jrborder
	PRIVATE
		src/jrborder_module.cpp
		src/jrborder.cpp
		src/jrborder.hpp
		src/plugininfo.hpp
		src/pluginRender.cpp
		../jrcommon/src/jrhelpers.cpp
		../jrcommon/src/jrhelpers.hpp
		../jrcommon/src/jrqthelpers.cpp
		../jrcommon/src/jrqthelpers.hpp
		../jrcommon/src/jrobshelpers.cpp
		../jrcommon/src/jrobshelpers.hpp
		../jrcommon/src/jrobsplugin.hpp
		../jrcommon/src/jrobsplugin.cpp
		../jrcommon/src/jrobsplugin_source.cpp
		../jrcommon/src/jrobsplugin_source.hpp
		../jrcommon/src/jrobsplugin_source_globalfuncs.hpp
		../jrcommon/src/jrobsplugin_options.cpp
		../jrcommon/src/jrobsplugin_options.hpp
		)
		

target_link_libraries(jrborder)

target_compile_definitions(
  jrborder PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrborder)
