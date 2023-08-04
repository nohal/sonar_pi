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

# -------- Build setup  --------
#
include(PluginOptions)
if (BUILD_TYPE)
  message(STATUS "Building: ${BUILD_TYPE}")
else ()
  message(STATUS "Configuring")
endif ()

# -------- Plugin setup ----------
#
include(Plugin.cmake)

# -------- Setup completed, build the plugin --------
#
project(${PKG_NAME} VERSION ${PKG_VERSION})
include(PluginCompiler)


set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})

add_library(${CMAKE_PROJECT_NAME} SHARED EXCLUDE_FROM_ALL ${SRC})
include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)

include(PluginCompiler)


add_subdirectory("opencpn-libs/${PKG_API_LIB}")
if(${PKG_NAME} MATCHES wx32)
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::api_wx32)
else()
  target_link_libraries(${CMAKE_PROJECT_NAME} ocpn::api)
endif()


set(VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(PACKAGE_NAME ${CMAKE_PROJECT_NAME})

# Create the minimim ocpn version id string like ov50
string(REGEX REPLACE "\([0-9]\)\.\([0-9]\).*" "ov\\1\\2"
       OCPN_MIN_VERSION "${API_OCPN_MIN_VERSION}")




#include_directories(${CMAKE_SOURCE_DIR}/include)
#include_directories(${CMAKE_BINARY_DIR}/include)

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





include(PluginSetup)



configure_file(${CMAKE_SOURCE_DIR}/config.h.in
               ${CMAKE_BINARY_DIR}/include/config.h)




if (COMMAND late_init)
  late_init()
endif ()


# Set up targets. Targets sets up new targets with BUILD_TYPE set to
# 'flatpak', 'android' or 'tarball'. The initial call without BUILD_TYPE ends
# here.
#
include(Targets)
# Add the primary build targets android, flatpak and tarball together
# with support targets. Parameters:
# - manifest: Flatpak build manifest

tarball_target()
add_custom_target(default ALL)
if("${ARM_ARCH}" STREQUAL "")
  add_dependencies(default tarball)
endif()


#if ("${BUILD_TYPE}" STREQUAL "")
#  return ()
#endif ()


if (APPLE)
  include(MacosWxwidgets)
endif ()
include(PluginInstall)
include(PluginLibs)

include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)

if (COMMAND add_plugin_libraries)
  add_plugin_libraries()
endif ()



configure_file(${CMAKE_SOURCE_DIR}/config.h.in
               ${CMAKE_BINARY_DIR}/include/config.h)


set(checksum "@checksum@")
configure_file(
  # The XML metadata file
  ${CMAKE_SOURCE_DIR}/plugin.xml.in
  ${CMAKE_BINARY_DIR}/${pkg_displayname}.xml.in
  @ONLY
)
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
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

add_executable(sonarsimulator WIN32 EXCLUDE_FROM_ALL ${CMAKE_SOURCE_DIR}/src/SonarSimulator.cpp)
target_sources(sonarsimulator PRIVATE ${SRC_SOSI})
target_link_libraries(sonarsimulator ${wxWidgets_LIBRARIES})

#target_link_libraries(${CMAKE_PROJECT_NAME} ${wxWidgets_LIBRARIES})