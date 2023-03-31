project(jryoutubechat)

# Find Qt
find_qt(COMPONENTS Core Widgets Svg Network)


add_library(jryoutubechat MODULE)

target_link_libraries(
jryoutubechat
PRIVATE
	OBS::libobs
	OBS::frontend-api
	Qt::Core
	Qt::Widgets
	Qt::Network
	)
	
	
	
	
  get_target_property(_PANEL_INCLUDE_DIRECTORY OBS::browser-panels
                      INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(jryoutubechat PRIVATE ${_PANEL_INCLUDE_DIRECTORY})
  target_compile_definitions(jryoutubechat PRIVATE BROWSER_AVAILABLE)

	


set_target_properties(jryoutubechat PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "Jr YouTube Id")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in jryoutubechat.rc)

target_sources(
	jryoutubechat
	PRIVATE
		jryoutubechat.rc
		src/jryoutubechat.cpp
		src/jryoutubechat.hpp
		src/jryoutubechatItems.cpp
		src/jryoutubechat_options.cpp
		src/jryoutubechat_options.hpp
		src/chatvote.hpp
		src/chatvote.cpp
		src/chatstats.hpp
		src/chatstats.cpp
		src/myQtItemDelegate.hpp
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

target_link_libraries(jryoutubechat)

target_compile_definitions(
  jryoutubechat PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(jryoutubechat)
