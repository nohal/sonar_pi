# ~~~
# Summary:     Add primary build targets
# License:     GPLv3+
# Copyright (c) 2020-2021 Alec Leamas
#
# Add the primary build targets android, flatpak and tarball together
# with helper targets. Also sets up the default target.
# ~~~

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.




if(TARGET tarball-build)
  return()
endif()

include(Metadata)

if(UNIX AND NOT APPLE AND NOT QT_ANDROID)
  set(_LINUX ON)
else()
  set(_LINUX OFF)
endif()

if(WIN32)
  if(CMAKE_VERSION VERSION_LESS 3.16)
    message(WARNING "windows requires cmake version 3.16 or higher")
  endif()
endif()

# Set up _build_cmd
set(_build_cmd
  cmake --build ${CMAKE_BINARY_DIR} --parallel ${OCPN_NPROC} --config $<CONFIG>
)

# Set up _build_target_cmd and _install_cmd
if(CMAKE_VERSION VERSION_LESS 3.16)
  set(_build_target_cmd make)
  set(_install_cmd make install)
else()
  set(_build_target_cmd
      cmake --build ${CMAKE_BINARY_DIR} --parallel ${OCPN_NPROC}
      --config $<CONFIG> --target
  )
  set(_install_cmd cmake --install ${CMAKE_BINARY_DIR} --config $<CONFIG>)
endif()

# Command to remove directory
if(CMAKE_VERSION VERSION_LESS 3.17)
  set(_rmdir_cmd "remove_directory")
else()
  set(_rmdir_cmd "rm -rf")
endif()


# Cmake batch file to compute and patch metadata checksum
#
set(_cs_script "
  execute_process(
    COMMAND  cmake -E sha256sum ${CMAKE_BINARY_DIR}/${pkg_tarname}.tar.gz
    OUTPUT_FILE ${CMAKE_BINARY_DIR}/${pkg_tarname}.sha256
  )
  file(READ ${CMAKE_BINARY_DIR}/${pkg_tarname}.sha256 _SHA256)
  string(REGEX MATCH \"^[^ ]*\" checksum \"\${_SHA256}\" )
  configure_file(
    ${CMAKE_BINARY_DIR}/${pkg_displayname}.xml.in
    ${CMAKE_BINARY_DIR}/${pkg_xmlname}.xml
    @ONLY
  )
")
file(WRITE "${CMAKE_BINARY_DIR}/checksum.cmake" ${_cs_script})

function(create_finish_script)
  set(_finish_script "
    execute_process(
      COMMAND cmake -E ${_rmdir_cmd} app/${pkg_displayname}
    )
    execute_process(
      COMMAND cmake -E rename app/files app/${pkg_displayname}
    )
    execute_process(
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/app
      COMMAND
        cmake -E
        tar -czf ../${pkg_tarname}.tar.gz --format=gnutar ${pkg_displayname}
    )
    message(STATUS \"Creating tarball ${pkg_tarname}.tar.gz\")

    execute_process(COMMAND cmake -P ${CMAKE_BINARY_DIR}/checksum.cmake)
    message(STATUS \"Computing checksum in ${pkg_xmlname}.xml\")
  ")
  file(WRITE "${CMAKE_BINARY_DIR}/finish_tarball.cmake" "${_finish_script}")
endfunction()


function(tarball_target)

  # tarball target setup
  #
  add_custom_command(
    OUTPUT tarball-conf-stamp
    COMMAND cmake -E touch tarball-conf-stamp
    COMMAND cmake
      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/app/files
      -DBUILD_TYPE:STRING=tarball
      $ENV{CMAKE_BUILD_OPTS}
      ${CMAKE_BINARY_DIR}
  )

  add_custom_target(tarball-build DEPENDS tarball-conf-stamp)
  add_custom_command(
    TARGET tarball-build COMMAND ${_build_target_cmd} ${PKG_NAME}
  )

  add_custom_target(tarball-install)
  add_custom_command(TARGET tarball-install COMMAND ${_install_cmd})

  create_finish_script()
  add_custom_target(tarball-finish)
  add_custom_command(
    TARGET tarball-finish      # Compute checksum
    COMMAND cmake -P ${CMAKE_BINARY_DIR}/finish_tarball.cmake
    VERBATIM
  )
  add_dependencies(tarball-install tarball-build)
  add_dependencies(tarball-finish tarball-install)

  add_custom_target(tarball)
  add_dependencies(tarball tarball-finish)
endfunction()


function(create_targets manifest)
  # Add the primary build targets android, flatpak and tarball together
  # with support targets. Parameters:
  # - manifest: Flatpak build manifest

  if(BUILD_TYPE STREQUAL "pkg")
    message(FATAL_ERROR "Legacy package generation is not supported.")
  endif()
  tarball_target()
  add_custom_target(default ALL)
  if("${ARM_ARCH}" STREQUAL "")
    add_dependencies(default tarball)
  endif()
endfunction()
