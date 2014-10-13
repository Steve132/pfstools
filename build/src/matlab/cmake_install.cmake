# Install script for directory: /Users/rafal/pfstools/src/matlab

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsclose.mexmaci64")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE SHARED_LIBRARY FILES "/Users/rafal/pfstools/build/src/matlab/pfsclose.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsclose.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsclose.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "pfsclose.mexmaci64"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsclose.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/rafal/pfstools/src/pfs"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsclose.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "$ORIGIN"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsclose.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsclose.mexmaci64")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsclose.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfsclose.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsopen.mexmaci64")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE SHARED_LIBRARY FILES "/Users/rafal/pfstools/build/src/matlab/pfsopen.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsopen.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsopen.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "pfsopen.mexmaci64"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsopen.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/rafal/pfstools/src/pfs"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsopen.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "$ORIGIN"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsopen.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsopen.mexmaci64")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsopen.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfsopen.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfspopen.mexmaci64")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE SHARED_LIBRARY FILES "/Users/rafal/pfstools/build/src/matlab/pfspopen.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspopen.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspopen.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "pfspopen.mexmaci64"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspopen.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/rafal/pfstools/src/pfs"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspopen.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "$ORIGIN"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspopen.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspopen.mexmaci64")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfspopen.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfspopen.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsput.mexmaci64")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE SHARED_LIBRARY FILES "/Users/rafal/pfstools/build/src/matlab/pfsput.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsput.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsput.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "pfsput.mexmaci64"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsput.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/rafal/pfstools/src/pfs"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsput.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "$ORIGIN"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsput.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsput.mexmaci64")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsput.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfsput.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsget.mexmaci64")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE SHARED_LIBRARY FILES "/Users/rafal/pfstools/build/src/matlab/pfsget.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsget.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsget.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "pfsget.mexmaci64"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsget.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/rafal/pfstools/src/pfs"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsget.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "$ORIGIN"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsget.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfsget.mexmaci64")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsget.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfsget.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_transform_colorspace.mexmaci64")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE SHARED_LIBRARY FILES "/Users/rafal/pfstools/build/src/matlab/pfs_transform_colorspace.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfs_transform_colorspace.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfs_transform_colorspace.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "pfs_transform_colorspace.mexmaci64"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfs_transform_colorspace.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/rafal/pfstools/src/pfs"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfs_transform_colorspace.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "$ORIGIN"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfs_transform_colorspace.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfs_transform_colorspace.mexmaci64")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_transform_colorspace.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_transform_colorspace.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfspclose.mexmaci64")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE SHARED_LIBRARY FILES "/Users/rafal/pfstools/build/src/matlab/pfspclose.mexmaci64")
  if(EXISTS "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspclose.mexmaci64" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspclose.mexmaci64")
    execute_process(COMMAND "/usr/bin/install_name_tool"
      -id "pfspclose.mexmaci64"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspclose.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -delete_rpath "/Users/rafal/pfstools/src/pfs"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspclose.mexmaci64")
    execute_process(COMMAND /usr/bin/install_name_tool
      -add_rpath "$ORIGIN"
      "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspclose.mexmaci64")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/usr/local/share/pfstools/matlab/pfspclose.mexmaci64")
    endif()
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfspclose.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfspclose.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/Contents.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/Contents.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_read_image.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_read_image.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_read_luminance.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_read_luminance.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_read_rgb.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_read_rgb.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_read_xyz.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_read_xyz.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_shell.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_shell.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_test_shell.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_test_shell.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_write_image.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_write_image.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_write_luminance.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_write_luminance.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_write_rgb.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_write_rgb.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfs_write_xyz.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfs_write_xyz.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsview.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfsview.m")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/pfstools/matlab/pfsview_rgb.m")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/local/share/pfstools/matlab" TYPE FILE FILES "/Users/rafal/pfstools/src/matlab/pfsview_rgb.m")
endif()

