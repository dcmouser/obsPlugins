project(jrytdock)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrytdock MODULE)

target_link_libraries(
jrytdock
PRIVATE
	OBS::libobs
	OBS::frontend-api
	Qt::Core
	Qt::Widgets
	Qt::Network
	PRIVATE CEF::Library	
	)
	
	
	
	
  get_target_property(_PANEL_INCLUDE_DIRECTORY OBS::browser-panels
                      INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(jrytdock PRIVATE ${_PANEL_INCLUDE_DIRECTORY})
  target_compile_definitions(jrytdock PRIVATE BROWSER_AVAILABLE)

	


set_target_properties(jrytdock PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "Jr YouTube Dock")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrytdock.rc)

target_sources(
	jrytdock
	PRIVATE
		jrytdock.rc
		src/jrytdock.cpp
		src/jrytdock.hpp
		src/jrytdock_options.cpp
		src/jrytdock_options.hpp
		../jrcommon/src/jrobshelpers.cpp
		../jrcommon/src/jrobshelpers.hpp
		../jrcommon/src/jrobsplugin.cpp
		../jrcommon/src/jrobsplugin.hpp
		../jrcommon/src/jrobsplugin_options.cpp
		../jrcommon/src/jrobsplugin_options.hpp
		src/plugininfo.hpp

		PRIVATE window-dock-browser.cpp window-dock-browser.hpp window-extra-browsers.cpp window-extra-browsers.hpp
		)

target_link_libraries(jrytdock)

target_compile_definitions(
  jrytdock PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrytdock)
