# ~~~
# Summary:      Main plugin build script
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.

# -------- Cmake setup ---------
#
cmake_minimum_required(VERSION 3.12.0)
include(${CMAKE_SOURCE_DIR}/cmake/CmakeSetup.cmake NO_POLICY_SCOPE)

include(PluginOptions)

set(PKG_NAME sonar_pi)
set(PKG_VERSION 0.5.1)
set(PKG_PRERELEASE "alpha")
set(PKG_API_LIB api-18)
set(PROJECT_VERSION_TWEAK ${PKG_PRERELEASE} )
set(DISPLAY_NAME Sonar) # Dialogs, installer artifacts, ...

set(PLUGIN_API_NAME Sonar) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "Sonar Display")
set(PKG_DESCRIPTION
    [=[
  Sonar Sensor Data Display
]=])

set(PKG_AUTHOR "Nicholas John Koch")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/aviatorhh/sonar_pi)
#set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/sonar.html)

project(${PKG_NAME} VERSION ${PKG_VERSION})

set(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(SRC 
        ${CMAKE_SOURCE_DIR}/src/serialib.cpp 
        ${CMAKE_SOURCE_DIR}/src/SerialDataReceiver.cpp           
        ${CMAKE_SOURCE_DIR}/src/UDPDataReceiver.cpp
        ${CMAKE_SOURCE_DIR}/src/SonarPane.cpp
        ${CMAKE_SOURCE_DIR}/src/SonarDisplayWindow.cpp
        ${CMAKE_SOURCE_DIR}/src/PreferencesWindow.cpp
        ${CMAKE_SOURCE_DIR}/src/sonar_pi.cpp
)


set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})

include(PluginCompiler)
add_library(${CMAKE_PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SRC})
include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)

#include(Targets)
#tarball_target()

add_subdirectory("opencpn-libs/${PKG_API_LIB}")
if(${PKG_NAME} MATCHES wx32)
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::api_wx32)
else()
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::api)
endif()
include(PluginLibs)

configure_file(${CMAKE_SOURCE_DIR}/config.h.in
               ${CMAKE_BINARY_DIR}/include/config.h)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# Create the minimim ocpn version id string like ov50
string(REGEX REPLACE "\([0-9]\)\.\([0-9]\).*" "ov\\1\\2"
       OCPN_MIN_VERSION "${API_OCPN_MIN_VERSION}")

#include(PluginInstall)

#
# Test application for development
#
#set(SRC_TEST
#  ${CMAKE_SOURCE_DIR}/src/serialib.cpp
#  ${CMAKE_SOURCE_DIR}/src/PreferencesWindowTest.cpp 
#)

#add_executable(test WIN32 ${CMAKE_SOURCE_DIR}/src/Test.cpp)
#target_sources(test PRIVATE ${SRC_TEST})
#target_link_libraries(test ${wxWidgets_LIBRARIES})


#
# Sonar Data Simulator
#
set(SRC_SOSI
  ${CMAKE_SOURCE_DIR}/include/SonarSimulator.h 
)

add_executable(sonarsimulator WIN32 ${CMAKE_SOURCE_DIR}/src/SonarSimulator.cpp)
target_sources(sonarsimulator PRIVATE ${SRC_SOSI})
target_link_libraries(sonarsimulator ${wxWidgets_LIBRARIES})

target_link_libraries(${CMAKE_PROJECT_NAME} ${wxWidgets_LIBRARIES})