project(jrtimestamps)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrtimestamps MODULE)

target_link_libraries(jrtimestamps PRIVATE OBS::libobs OBS::frontend-api Qt::Core Qt::Widgets Qt::Network)

set_target_properties(jrtimestamps PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "YouTube Timestamp File Writer")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrtimestamps.rc)

target_sources(
	jrtimestamps
	PRIVATE
		jrtimestamps.rc
		src/jrtimestamps.cpp
		src/jrtimestamps.hpp
		src/jrtimestamps_options.cpp
		src/jrtimestamps_options.hpp
		../jrcommon/src/jrobshelpers.cpp
		../jrcommon/src/jrobshelpers.hpp
		../jrcommon/src/jrobsplugin.cpp
		../jrcommon/src/jrobsplugin.hpp
		../jrcommon/src/jrobsplugin_options.cpp
		../jrcommon/src/jrobsplugin_options.hpp
		src/plugininfo.hpp
		)

target_link_libraries(jrtimestamps)

target_compile_definitions(
  jrtimestamps PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrtimestamps)
