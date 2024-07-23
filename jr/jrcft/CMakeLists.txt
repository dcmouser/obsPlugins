project(jrcft)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrcft MODULE)

target_link_libraries(
jrcft
PRIVATE
	OBS::libobs
	OBS::frontend-api
	Qt::Core
	Qt::Widgets
	Qt::Network
	)
	
	
	
	
  get_target_property(_PANEL_INCLUDE_DIRECTORY OBS::browser-panels
                      INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(jrcft PRIVATE ${_PANEL_INCLUDE_DIRECTORY})
  target_compile_definitions(jrcft PRIVATE BROWSER_AVAILABLE)

	


set_target_properties(jrcft PROPERTIES FOLDER "plugins/jr")
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)


set(MODULE_DESCRIPTION "jrCft")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrcft.rc)

target_sources(
	jrcft
	PRIVATE
		src/jrcft.cpp
		src/jrcft.hpp
		src/jrcft_options.cpp
		src/jrcft_options.hpp
		src/jrcft_work.cpp
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
		src/plugininfo.hpp
		)

target_link_libraries(jrcft)

target_compile_definitions(
  jrcft PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrcft)
