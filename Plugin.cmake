

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

set(SRC 
  ${CMAKE_SOURCE_DIR}/src/serialib.cpp 
  ${CMAKE_SOURCE_DIR}/src/SerialDataReceiver.cpp           
  ${CMAKE_SOURCE_DIR}/src/UDPDataReceiver.cpp
  ${CMAKE_SOURCE_DIR}/src/SonarPane.cpp
  ${CMAKE_SOURCE_DIR}/src/SonarDisplayWindow.cpp
  ${CMAKE_SOURCE_DIR}/src/PreferencesWindow.cpp
  ${CMAKE_SOURCE_DIR}/src/sonar_pi.cpp
)
        


macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers
  # and ocpn::api is available.

  # Fix OpenGL deprecated warnings in Xcode
  target_compile_definitions(${PACKAGE_NAME} PRIVATE GL_SILENCE_DEPRECATION)

  target_include_directories(${PACKAGE_NAME} PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/include 
  )
endmacro ()