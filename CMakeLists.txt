cmake_minimum_required(VERSION 3.14.0)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "")

set(vst3sdk_SOURCE_DIR "C:/Users/jul/VST_SDK/vst3sdk")
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(philter
    # This is your plug-in version number. Change it here only.
    # Version number symbols usable in C++ can be found in
    # source/version.h and ${PROJECT_BINARY_DIR}/projectversion.h.
    VERSION 1.0.0.0 
    DESCRIPTION "philter VST 3 Plug-in"
)

set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(philter
    source/version.h
    source/ph_cids.h
    source/ph_processor.h
    source/ph_processor.cpp
    source/ph_controller.h
    source/ph_controller.cpp
    source/ph_entry.cpp
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
    target_sources(philter
        PRIVATE
            resource/ph_editor.uidesc
    )
    target_link_libraries(philter
        PRIVATE
            vstgui_support
    )
    smtg_target_add_plugin_resources(philter
        RESOURCES
            "resource/ph_editor.uidesc"
    )
endif(SMTG_ADD_VSTGUI)
# -------------------

smtg_target_add_plugin_snapshots (philter
    RESOURCES
        resource/D58085F611BE510EBAC77571E1491EE1_snapshot.png
        resource/D58085F611BE510EBAC77571E1491EE1_snapshot_2.0x.png
)

target_link_libraries(philter
    PRIVATE
        sdk
)

smtg_target_configure_version_file(philter)

if(SMTG_MAC)
    smtg_target_set_bundle(philter
        BUNDLE_IDENTIFIER xyz.jul.philter
        COMPANY_NAME "jul"
    )
    smtg_target_set_debug_executable(philter
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(philter PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT philter)

        smtg_target_set_debug_executable(philter
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif(SMTG_MAC)
