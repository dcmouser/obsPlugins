project(jrAutoAspect)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jrAutoAspect MODULE)

target_link_libraries(jrAutoAspect PRIVATE OBS::libobs OBS::frontend-api Qt::Core Qt::Widgets Qt::Network)

set_target_properties(jrAutoAspect PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "OBS Filter auto switches source aspect ratio")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jrAutoAspect.rc)

target_sources(
	jrAutoAspect
	PRIVATE
		src/jrautoaspect.cpp
		src/jrautoaspect.hpp
		src/plugininfo.hpp
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
		

target_link_libraries(jrAutoAspect)

target_compile_definitions(
  jrAutoAspect PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jrAutoAspect)
