cmake_minimum_required (VERSION 2.8.8)
project(pfstools)

set(CPACK_GENERATOR TGZ)
set(CPACK_SOURCE_GENERATOR TGZ)
# CPack version numbers for release tarball name.
set(CPACK_PACKAGE_VERSION_MAJOR 2)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_SOURCE_IGNORE_FILES
"~$"
"\\\\.cvsignore$"
"^${PROJECT_SOURCE_DIR}.*/CVS/"
"^${PROJECT_SOURCE_DIR}/debian/"
"^${PROJECT_SOURCE_DIR}/old/"
)
include( CPack )

#SET(ZLIB_DIR "e:/zlib/")
#SET(CMAKE_INCLUDE_PATH ${ZLIB_DIR}/include ${CMAKE_INCLUDE_PATH})
#SET(CMAKE_LIBRARY_PATH ${ZLIB_DIR}/ ${ZLIB_DIR}/lib ${CMAKE_LIBRARY_PATH})

if( "${CYGWIN}" STREQUAL "" )
  set(CYGWIN 0)  
endif( "${CYGWIN}" STREQUAL "" )

set (pfstools_VERSION_MAJOR 2)
set (pfstools_VERSION_MINOR 0.0)
set (pfslib_version 1.2)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)
 
set (MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man/man1" CACHE PATH "The directory where the man pages are")

include( CheckCXXSourceCompiles )


# ======== Check for OpenMP =======

find_package(OpenMP)

# ======== Has branch prediction =======

check_cxx_source_compiles( "int main() { int x = 0; if( __builtin_expect((x),0) ) x = 1; return 0; }" HAS_BRANCH_PEDICTION )

if( HAS_BRANCH_PREDICTION )
  set( BRANCH_PREDICTION 1 )
else( HAS_BRANCH_PREDICTION )
  set( BRANCH_PREDICTION 0 )
endif( HAS_BRANCH_PREDICTION )

# ======== Find bash =======

# TODO: What if the check fails
find_program (BASH_EXECUTABLE bash)
message( "Using bash: ${BASH_EXECUTABLE}" )

# ======== Build static or dynamic libraries =======

OPTION (BUILD_SHARED_LIBS "Build pfs as a shared library" OFF)
# TODO: Shared libs do not work with VS. Add declspec as explained at: http://www.cmake.org/Wiki/BuildingWinDLL
# Set the LIB_TYPE variable to STATIC
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  SET (LIB_TYPE SHARED)
ENDIF (BUILD_SHARED_LIBS)

# ======== Check for getopt, which is missing on Windows ========

check_cxx_source_compiles( "#include \"getopt.h\" \n int main() { return 0; }" HAS_GETOPT )
if( NOT HAS_GETOPT )
  MESSAGE( STATUS "getopt not found, a replacement needs to be compiled. " )
  set( GETOPT_INCLUDE "${PROJECT_SOURCE_DIR}/src/getopt" )
  set( GETOPT_OBJECT "$<TARGET_OBJECTS:getopt>" )
else( NOT HAS_GETOPT )
  MESSAGE( STATUS "getopt found. " )
  set( GETOPT_INCLUDE ".") # work around, cannot pass empty string
  set( GETOPT_OBJECT )
endif( NOT HAS_GETOPT )

set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

# ======== OpenEXR ===========

OPTION(WITH_OpenEXR "Compile with OpenEXR library" ON)

if( WITH_OpenEXR )

find_package (OpenEXR)
if( NOT OPENEXR_FOUND )
MESSAGE( STATUS "OpenEXR not found. The following command will not be
compiled: pfsinexr pfsoutexr. " )
endif( NOT OPENEXR_FOUND )

endif( WITH_OpenEXR )

# ======== Image Magick ===========

OPTION(WITH_ImageMagick "Use ImageMagick library" ON)

if( WITH_ImageMagick )

find_package(ImageMagick COMPONENTS Magick++ MagickCore)
if( NOT ImageMagick_FOUND )
MESSAGE( STATUS "ImageMagick not found. The following commands will not be
compiled: pfsinimgmagick pfsoutimgmagick pfsouthdrhtml. " )
endif( NOT ImageMagick_FOUND )

else( WITH_ImageMagick )

      set( ImageMagick_FOUND OFF )

endif( WITH_ImageMagick )

message( "ImageMagick CFLAGS: " ${ImageMagick_INCLUDE_DIRS} )

# ======== NetPBM ===========

#include( ${PROJECT_SRC_DIR}/cmake/FindNETPBM.cmake )

OPTION(WITH_NetPBM "Use NetPBM library" ON)

if( WITH_NetPBM )

    find_package(NETPBM)
    if( NOT NETPBM_FOUND )
    	MESSAGE( STATUS "NetPBM not found. The following commands will
    not be compiled: pfsinppm pfsoutppm. " )
    endif( NOT NETPBM_FOUND )

else( WITH_NetPBM )

      set( NETPBM_FOUND OFF )

endif( WITH_NetPBM )

# ======== TIFF ===========

#include( ${PROJECT_SRC_DIR}/cmake/FindTIDD.cmake )

OPTION(WITH_TIFF "Use TIFF library" ON)

if( WITH_TIFF )

    find_package(TIFF)
    if( NOT TIFF_FOUND )
    	MESSAGE( STATUS "TIFF not found. The following commands will
    not be compiled: pfsintiff pfsouttiff. " )
    endif( NOT TIFF_FOUND )

else( WITH_TIFF )

      set( NETPBM_FOUND OFF )

endif( WITH_TIFF )

# ======== QT ===========

OPTION(WITH_QT "Use QT library" ON)

if( WITH_QT )

    find_package(Qt4)
    if( NOT QT_FOUND )
    	MESSAGE( STATUS "Qt4 library not found. The following commands
    will not be compiled: pfsview. " )
    else( NOT QT_FOUND )
        MESSAGE( STATUS "Qt4 found" )
    endif( NOT QT_FOUND )

else( WITH_QT )

      set( QT_FOUND OFF )

endif( WITH_QT )

# ======== Matlab ===========

OPTION(WITH_MATLAB "Compile Matlab MEX files" ON)

SET (MATLAB_DEST_DIR "${CMAKE_INSTALL_PREFIX}/share/pfstools/matlab"
CACHE PATH  "Copy matlab functions to this directory")

if( WITH_MATLAB )
  find_package(MATLAB)

  if( MATLAB_FOUND )
    message(STATUS "matlab found.")
  elseif( MATLAB_FOUND )
    message( STATUS "Matlab or mex compiler not found. Matlab integration will not be compiled." )
  endif( MATLAB_FOUND )
  
endif( WITH_MATLAB )


#include( FindPkgConfig.cmake )

# ======== OpenGL ===========

OPTION(WITH_pfsglview "Compile pfsglview, requires OpenGL and GLUT" ON)

if( WITH_pfsglview )

find_path(OPENGL_INCLUDE_DIR GL/gl.h )

find_package(OpenGL )
find_package(GLUT )

if( NOT OPENGL_FOUND )
	MESSAGE( STATUS "OpenGL not found. The following command will not be
compiled: pfsglview. " )
endif( NOT OPENGL_FOUND )

if( NOT GLUT_FOUND )
	MESSAGE( STATUS "GLUT not found. The following command will not be
compiled: pfsglview. " )
endif( NOT GLUT_FOUND )

endif( WITH_pfsglview )

# ========= FFTW  ===========

OPTION(WITH_fftw "Use fftw to speed up some operations" ON)

if( WITH_fftw )

find_package (FFTW)

if( NOT FFTW_FOUND )
	MESSAGE( STATUS "FFTW library not found. Acceleration for pfstmo_durand02 disabled. " )
	set( HAVE_FFTW3F 0 )
else( NOT FFTW_FOUND )
	set( HAVE_FFTW3F 1 )
endif( NOT FFTW_FOUND )

else( WITH_fftw )
	set( HAVE_FFTW3F 0 )
endif( WITH_fftw )



# ======== HDRVC ===========
# This is an experimental implementation of HDR-MPEG codec, not included in the official release.
OPTION(WITH_HDRVC "Compile HDRVC (internal release only)" OFF)

# ======== MKOCTFILE/OCTAVE ===========

#OPTION(WITH_OCTAVE "Compile Octave files" ON)

SET (OCTAVE_DEST_DIR "${CMAKE_INSTALL_PREFIX}/share/pfstools/octave"
CACHE PATH  "Copy octave functions to this directory")

find_program(MKOCTFILE mkoctfile)
if(NOT MKOCTFILE)
  message(STATUS "Failed to find mkoctfile. Maybe liboctave-dev is not installed.")
endif()
if(MKOCTFILE)
	message(STATUS "mkoctfile found.")
endif()

# ======== Config and sub dirs ===========

configure_file (
  "${PROJECT_SOURCE_DIR}/cmake_config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

add_subdirectory (src) 
