project(jrstats)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrstats MODULE)

target_link_libraries(jrstats PRIVATE OBS::libobs OBS::frontend-api Qt::Core Qt::Widgets Qt::Network)

set_target_properties(jrstats PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "Replacement Stats Dock")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrstats.rc)

target_sources(
	jrstats
	PRIVATE
		src/plugininfo.hpp
		src/jrstats.cpp
		src/jrstats.hpp
		src/jrstats_options.cpp
		src/jrstats_options.hpp
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

target_link_libraries(jrstats)

target_compile_definitions(
  jrstats PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrstats)
