if(OS_WINDOWS)
endif()

include(cmake/ObsPluginHelpers.cmake)


add_subdirectory(obs-textml)
add_subdirectory(transition-fadeDelayed)
add_subdirectory(ObsAutoZoom)
add_subdirectory(jrScreenFlip)
add_subdirectory(jrstats)
add_subdirectory(jrtimestamps)
add_subdirectory(jrnotedock)
add_subdirectory(jryoutubechat)
add_subdirectory(jrcft)
add_subdirectory(jrborder)
add_subdirectory(jrAutoAspect)
add_subdirectory(jrdockie)
