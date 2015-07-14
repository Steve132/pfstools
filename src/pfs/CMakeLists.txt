
include_directories ("${PROJECT_BINARY_DIR}/")
if( NOT HAS_GETOPT )
	include_directories ("${GETOPT_INCLUDE}")
endif( NOT HAS_GETOPT )
add_library(pfs ${LIB_TYPE} colorspace.cpp pfs.cpp pfsutils.cpp array2d.h pfs.h "${GETOPT_OBJECT}")

# SOVERSION changes only when the library API changes, so it may be
# different from the package version
set_target_properties(pfs PROPERTIES VERSION 2.0.0 SOVERSION 2)

# TODO: Make it platform dependent - only GCC linux / perhaps Mac
# This is needed when linking with matlab mex files
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fPIC" )
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC" )

# Replace a few tags in pfs.pc.in file, create pfs.pc
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/pfs.pc.in file_content)
#message("Input:\n${file_content}")
set (file_content_res)
string(REGEX REPLACE "(@prefix@)" "${CMAKE_INSTALL_PREFIX}" file_content_res "${file_content}")
string(REGEX REPLACE "(@PACKAGE_VERSION@)" "${pfstools_VERSION_MAJOR}.${pfstools_VERSION_MINOR}" file_content_res "${file_content_res}")
#message("Output:\n${file_content_res}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/pfs.pc" "${file_content_res}")	

install (TARGETS pfs 
	LIBRARY DESTINATION lib${LIB_SUFFIX}
	ARCHIVE DESTINATION lib${LIB_SUFFIX})

#install (FILES ${CMAKE_CURRENT_BINARY_DIR}/cygpfs.dll DESTINATION bin)
install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/pfs.h ${CMAKE_CURRENT_SOURCE_DIR}/array2d.h DESTINATION include/pfs)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/pfs.pc DESTINATION lib${LIB_SUFFIX}/pkgconfig)
	
#install (FILES pfsoutppm.1 DESTINATION ${MAN_DIR})


