#/****************************************************************************
# Copyright (c) 2013-2014 cocos2d-x.org
# Copyright (c) 2015-2017 Chukong Technologies Inc.
#
# http://www.cocos2d-x.org
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# ****************************************************************************/

cmake_minimum_required(VERSION 3.6)

set(APP_NAME Clash_of_Clans)

project(${APP_NAME})

option(USE_COCOS_ENGINE "Build with real cocos2d engine" OFF)
option(BUILD_APP "Build game application target" ${USE_COCOS_ENGINE})
option(BUILD_TESTS "Build mock tests" ON)

if(XCODE)
    if(NOT DEFINED CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET)
        SET (CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0)
    endif()
endif()

if(USE_COCOS_ENGINE AND NOT DEFINED BUILD_ENGINE_DONE) # to test Clash_of_Clans into root project
    set(COCOS2DX_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cocos2d)
    set(CMAKE_MODULE_PATH ${COCOS2DX_ROOT_PATH}/cmake/Modules/)

    include(CocosBuildSet)
    add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos ${ENGINE_BINARY_PATH}/cocos/core)
endif()

# record sources, headers, resources...
set(GAME_SOURCE)
set(GAME_HEADER)

set(GAME_RES_FOLDER
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources"
    )
if(USE_COCOS_ENGINE AND (APPLE OR WINDOWS) AND COMMAND cocos_mark_multi_resources)
    cocos_mark_multi_resources(common_res_files RES_TO "Resources" FOLDERS ${GAME_RES_FOLDER})
endif()

# Automatic collector
file(GLOB_RECURSE GAME_SOURCE "Classes/*.cpp")
file(GLOB_RECURSE GAME_HEADER "Classes/*.h")
# Exclude mocks/tests from the app target sources.
list(FILTER GAME_SOURCE EXCLUDE REGEX "Classes/Mocks/")
list(FILTER GAME_HEADER EXCLUDE REGEX "Classes/Mocks/")

if(ANDROID)
    # change APP_NAME to the share library name for Android, it's value depend on AndroidManifest.xml
    set(APP_NAME MyGame)
    list(APPEND GAME_SOURCE
         proj.android/app/jni/hellocpp/main.cpp
         )
elseif(LINUX)
    list(APPEND GAME_SOURCE
         proj.linux/main.cpp
         )
elseif(WINDOWS)
    list(APPEND GAME_HEADER
         proj.win32/main.h
         proj.win32/resource.h
         )
    list(APPEND GAME_SOURCE
         proj.win32/main.cpp
         proj.win32/game.rc
         ${common_res_files}
         )
elseif(APPLE)
    if(IOS)
        list(APPEND GAME_HEADER
             proj.ios_mac/ios/AppController.h
             proj.ios_mac/ios/RootViewController.h
             )
        set(APP_UI_RES
            proj.ios_mac/ios/LaunchScreen.storyboard
            proj.ios_mac/ios/LaunchScreenBackground.png
            proj.ios_mac/ios/Images.xcassets
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/ios/main.m
             proj.ios_mac/ios/AppController.mm
             proj.ios_mac/ios/RootViewController.mm
             proj.ios_mac/ios/Prefix.pch
             ${APP_UI_RES}
             )
    elseif(MACOSX)
        set(APP_UI_RES
            proj.ios_mac/mac/Icon.icns
            proj.ios_mac/mac/Info.plist
            )
        list(APPEND GAME_SOURCE
             proj.ios_mac/mac/main.cpp
             proj.ios_mac/mac/Prefix.pch
             ${APP_UI_RES}
             )
    endif()
    list(APPEND GAME_SOURCE ${common_res_files})
endif()

# mark app complie info and libs info
set(all_code_files
    ${GAME_HEADER}
    ${GAME_SOURCE}
    )
if(BUILD_APP)
    if(NOT ANDROID)
        add_executable(${APP_NAME} ${all_code_files})
    else()
        add_library(${APP_NAME} SHARED ${all_code_files})
        add_subdirectory(${COCOS2DX_ROOT_PATH}/cocos/platform/android ${ENGINE_BINARY_PATH}/cocos/platform)
        target_link_libraries(${APP_NAME} -Wl,--whole-archive cpp_android_spec -Wl,--no-whole-archive)
    endif()
    if(USE_COCOS_ENGINE)
        target_compile_definitions(${APP_NAME} PRIVATE USE_COCOS_ENGINE=1)
    else()
        target_compile_definitions(${APP_NAME} PRIVATE USE_COCOS_ENGINE=0)
    endif()
    # Keep "Classes/..." includes working by adding the repo root to include paths.
    if(USE_COCOS_ENGINE)
        target_link_libraries(${APP_NAME} cocos2d)
        target_include_directories(${APP_NAME}
            PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                PRIVATE Classes
                PRIVATE Classes/Core
                PRIVATE Classes/Entities
                PRIVATE Classes/Map
                PRIVATE Classes/Managers
                PRIVATE Classes/Main
                PRIVATE Classes/UI
                PRIVATE Classes/Scenes
                PRIVATE ${COCOS2DX_ROOT_PATH}/cocos/audio/include/
        )

        # mark app resources
        setup_cocos_app_config(${APP_NAME})
        if(APPLE)
            set_target_properties(${APP_NAME} PROPERTIES RESOURCE "${APP_UI_RES}")

            if(MACOSX)
                set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/mac/Info.plist")
            elseif(IOS)
                set_xcode_property(${APP_NAME} INFOPLIST_FILE "${CMAKE_CURRENT_SOURCE_DIR}/proj.ios_mac/ios/Info.plist")
                set_xcode_property(${APP_NAME} ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon")
            endif()

            # For code-signing, set the DEVELOPMENT_TEAM:
            #set_xcode_property(${APP_NAME} DEVELOPMENT_TEAM "GRLXXXX2K9")
        elseif(WINDOWS)
            cocos_copy_target_dll(${APP_NAME})
        endif()

        if(LINUX OR WINDOWS)
            cocos_get_resource_path(APP_RES_DIR ${APP_NAME})
            cocos_copy_target_res(${APP_NAME} LINK_TO ${APP_RES_DIR} FOLDERS ${GAME_RES_FOLDER})
        endif()
    endif()
endif()

if(BUILD_TESTS)
    enable_testing()
    add_library(cocos_stub INTERFACE)
    target_include_directories(cocos_stub INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/Classes/Mocks/CocosShim
    )

    add_executable(mock_tests
            Classes/Mocks/EngineMock/MapLayer.cpp
            Classes/Mocks/EngineMock/MouseController.cpp
            Classes/Mocks/EngineMock/TileHighlighter.cpp
            Classes/Mocks/EngineMock/TilePlacementController.cpp
            Classes/Mocks/GameplayMock/Building.cpp
            Classes/Mocks/GameplayMock/BaseEntity.cpp
            Classes/Mocks/GameplayMock/CombatResolver.cpp
            Classes/Mocks/GameplayMock/CostQuery.cpp
            Classes/Mocks/GameplayMock/EconomySystem.cpp
            Classes/Mocks/GameplayMock/GameEvents.cpp
            Classes/Mocks/GameplayMock/HealthComp.cpp
            Classes/Mocks/GameplayMock/TroopCommandService.cpp
            Classes/Mocks/GameplayMock/Unit.cpp
            Classes/Integration/Engine/InputRouterImpl.cpp
            Classes/Integration/Engine/CocosAudioSink.cpp
            Classes/Integration/SceneFlowServiceImpl.cpp
            Classes/Mocks/IntegrationMock/PlayerIdentityService.cpp
            Classes/Mocks/IntegrationMock/SceneFlowService.cpp
            Classes/Managers/AudioManager/AudioManager.cpp
            Classes/Core/GameConfig.cpp
            Classes/Scenes/BootScene.cpp
            Classes/Scenes/GameStageScene.cpp
            Classes/Scenes/MenuScene.cpp
            Classes/Scenes/ResultsScene.cpp
            Classes/Integration/GameServices.cpp
            Classes/Mocks/Tests/MockTests.cpp
    )
    target_include_directories(mock_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            Classes
            Classes/Contract
            Classes/Mocks/CocosShim
    )
    target_compile_definitions(mock_tests PRIVATE USE_COCOS_ENGINE=0)
    target_link_libraries(mock_tests PRIVATE cocos_stub)

    add_test(NAME mock_tests COMMAND mock_tests)
    add_test(NAME include_path_lint COMMAND bash ${CMAKE_SOURCE_DIR}/scripts/check_includes.sh)

    add_library(app_shell STATIC
            Classes/Main/AppDelegate.cpp
    )
    target_include_directories(app_shell PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            Classes
            Classes/Contract
            Classes/Mocks/CocosShim
    )
    target_compile_definitions(app_shell PRIVATE USE_COCOS_ENGINE=0)
    target_link_libraries(app_shell PRIVATE cocos_stub)
endif()
