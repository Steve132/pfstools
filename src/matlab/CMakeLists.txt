include_directories ("${PROJECT_BINARY_DIR}/" "${PROJECT_SOURCE_DIR}/src/pfs")

#include_directories ("${PROJECT_BINARY_DIR}/" "${PROJECT_SOURCE_DIR}/src/pfs" ${MATLAB_INCLUDE_DIR})

link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

execute_process(COMMAND ${MATLAB_ROOT}/bin/mexext OUTPUT_VARIABLE MEX_EXT OUTPUT_STRIP_TRAILING_WHITESPACE)

message( "Matlab: mexext: ${MEX_EXT}" )
set(SRCMEX pfsclose.cpp pfsopen.cpp pfspopen.cpp pfsput.cpp pfsget.cpp pfs_transform_colorspace.cpp pfspclose.cpp) 

set(SRCM Contents.m pfs_read_image.m pfs_read_luminance.m
pfs_read_rgb.m pfs_read_xyz.m pfs_shell.m pfs_test_shell.m
pfs_write_image.m pfs_write_luminance.m pfs_write_rgb.m
pfs_write_xyz.m pfsview.m pfsview_rgb.m ) 

cmake_policy(SET CMP0018 OLD)

SET( CMAKE_CXX_COMPILER ${MATLAB_MEX} )
SET( CMAKE_C_COMPILER   ${MATLAB_MEX} )
# Suffix and Prefix of the output target file
SET( CMAKE_SHARED_LIBRARY_SUFFIX ".${MEX_EXT}" )  # set suffix to .mexa64
SET( CMAKE_SHARED_LIBRARY_PREFIX )      # remove the "lib" prefix
 
# Variables controlling the build-phrase
SET( CMAKE_CXX_FLAGS "-cxx -largeArrayDims" )
#SET( CMAKE_CXX_FLAGS "-cxx -largeArrayDims CXXFLAGS='$$CXXFLAGS -std=c++11'" )
SET( CMAKE_SHARED_LIBRARY_CXX_FLAGS )       # remove the -fPIC option. mex does not accept the "-fPIC" option
 
SET( CMAKE_CXX_COMPILE_OBJECT 
    "<CMAKE_CXX_COMPILER> <DEFINES> <FLAGS> -outdir <OBJECT_DIR> -c <SOURCE>; mv <OBJECT_DIR>/$$(basename <SOURCE> .cpp ).o <OBJECT>"
#    "<CMAKE_CXX_COMPILER> <DEFINES> <FLAGS> -outdir <OBJECT_DIR> -c <SOURCE>"
)
 
# Variables controlling the linking-phase
SET( CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS )    # remove -shared options. mex does not accept the "-shared" option
 
SET(
    CMAKE_CXX_CREATE_SHARED_LIBRARY
    "<CMAKE_CXX_COMPILER> -cxx <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> -output <TARGET> <OBJECTS> <LINK_LIBRARIES>"
)

SET( CMAKE_SKIP_RPATH TRUE ) 
# Variables controlling the installation RPATH
SET( CMAKE_INSTALL_RPATH "\$ORIGIN" )
# CMake will reset RPATH at the installation phase, so we need to specify CMAKE_INSTALL_RPATH
 
MESSAGE( STATUS "mex.cmake is loaded.\n" )
 
 
## To add an mex file, you can:
#  ADD_LIBRARY( myprogram SHARED myprogram.cxx )
## Make sure you specify it as SHARED, and make sure you use the cxx extension. 

add_definitions(-DMATLAB_MEX_FILE) 
foreach(SRC ${SRCMEX}) 
  get_filename_component(MEXNAME ${SRC} NAME_WE) 
  add_library(${MEXNAME} SHARED ${SRC} mex_utils.cpp ${MEXDEFFILE}) 
#  target_link_libraries(${MEXNAME} pfs ${MATLAB_LIBRARIES}) 
  target_link_libraries(${MEXNAME} pfs ) 
#  set_target_properties(${MEXNAME} PROPERTIES PREFIX "" SUFFIX 
#".${MEX_EXT}") 

  install (TARGETS ${MEXNAME} DESTINATION ${MATLAB_DEST_DIR})
  install (FILES ${MEXNAME}.m DESTINATION ${MATLAB_DEST_DIR})

endforeach(SRC) 

foreach(SRC ${SRCM}) 
  install (FILES ${SRC} DESTINATION ${MATLAB_DEST_DIR})
endforeach(SRC) 
