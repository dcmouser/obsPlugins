project(jrnotedock)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrnotedock MODULE)

target_link_libraries(
jrnotedock
PRIVATE
	OBS::libobs
	OBS::frontend-api
	Qt::Core
	Qt::Widgets
	Qt::Network
	)
	
	
	
	
  get_target_property(_PANEL_INCLUDE_DIRECTORY OBS::browser-panels
                      INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(jrnotedock PRIVATE ${_PANEL_INCLUDE_DIRECTORY})
  target_compile_definitions(jrnotedock PRIVATE BROWSER_AVAILABLE)

	


set_target_properties(jrnotedock PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "Jr Note Dock")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrnotedock.rc)

target_sources(
	jrnotedock
	PRIVATE
		jrnotedock.rc
		src/jrnotedock.cpp
		src/jrnotedock.hpp
		src/jrnotedock_options.cpp
		src/jrnotedock_options.hpp
		../jrcommon/src/jrobshelpers.cpp
		../jrcommon/src/jrobshelpers.hpp
		../jrcommon/src/jrobsplugin.cpp
		../jrcommon/src/jrobsplugin.hpp
		../jrcommon/src/jrobsplugin_options.cpp
		../jrcommon/src/jrobsplugin_options.hpp
		src/plugininfo.hpp
		)

target_link_libraries(jrnotedock)

target_compile_definitions(
  jrnotedock PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrnotedock)
