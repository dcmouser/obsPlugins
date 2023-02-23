project(jryoutubeid)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jryoutubeid MODULE)

target_link_libraries(
jryoutubeid
PRIVATE
	OBS::libobs
	OBS::frontend-api
	Qt::Core
	Qt::Widgets
	Qt::Network
	)
	
	
	
	
  get_target_property(_PANEL_INCLUDE_DIRECTORY OBS::browser-panels
                      INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(jryoutubeid PRIVATE ${_PANEL_INCLUDE_DIRECTORY})
  target_compile_definitions(jryoutubeid PRIVATE BROWSER_AVAILABLE)

	


set_target_properties(jryoutubeid PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "Jr YouTube Id")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jryoutubeid.rc)

target_sources(
	jryoutubeid
	PRIVATE
		jryoutubeid.rc
		src/jryoutubeid.cpp
		src/jryoutubeid.hpp
		src/jryoutubeid_options.cpp
		src/jryoutubeid_options.hpp
		../jrcommon/src/jrobshelpers.cpp
		../jrcommon/src/jrobshelpers.hpp
		../jrcommon/src/jrobsplugin.cpp
		../jrcommon/src/jrobsplugin.hpp
		../jrcommon/src/jrobsplugin_options.cpp
		../jrcommon/src/jrobsplugin_options.hpp
		src/plugininfo.hpp
		)

target_link_libraries(jryoutubeid)

target_compile_definitions(
  jryoutubeid PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jryoutubeid)
