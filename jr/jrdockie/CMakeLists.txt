project(jrdockie)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrdockie MODULE)

target_link_libraries(
jrdockie
PRIVATE
	OBS::libobs
	OBS::frontend-api
	Qt::Core
	Qt::Widgets
	Qt::Network
	)
	
	
	
	
  get_target_property(_PANEL_INCLUDE_DIRECTORY OBS::browser-panels
                      INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(jrdockie PRIVATE ${_PANEL_INCLUDE_DIRECTORY} )
  target_compile_definitions(jrdockie PRIVATE BROWSER_AVAILABLE)

	


set_target_properties(jrdockie PROPERTIES FOLDER "plugins/jr")
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)



set(MODULE_DESCRIPTION "Jr Dock IO")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrdockie.rc)

target_sources(
	jrdockie
	PRIVATE
		jrdockie.rc
		src/jrdockie.cpp
		src/jrdockie.hpp
		src/jrdockie_options.cpp
		src/jrdockie_options.hpp
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

target_link_libraries(jrdockie)





target_compile_definitions(
  jrdockie PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrdockie)
