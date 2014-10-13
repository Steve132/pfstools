# Install script for directory: /Users/rafal/pfstools/src/tmo

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

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/rafal/pfstools/build/src/tmo/drago03/cmake_install.cmake")
  include("/Users/rafal/pfstools/build/src/tmo/mantiuk06/cmake_install.cmake")
  include("/Users/rafal/pfstools/build/src/tmo/reinhard02/cmake_install.cmake")
  include("/Users/rafal/pfstools/build/src/tmo/reinhard05/cmake_install.cmake")
  include("/Users/rafal/pfstools/build/src/tmo/pattanaik00/cmake_install.cmake")
  include("/Users/rafal/pfstools/build/src/tmo/fattal02/cmake_install.cmake")
  include("/Users/rafal/pfstools/build/src/tmo/durand02/cmake_install.cmake")
  include("/Users/rafal/pfstools/build/src/tmo/mai11/cmake_install.cmake")

endif()

