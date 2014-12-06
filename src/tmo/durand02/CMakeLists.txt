include_directories ("${PROJECT_BINARY_DIR}/"
"${PROJECT_SOURCE_DIR}/src/pfs" "${PROJECT_SOURCE_DIR}/src/tmo/pfstmo")
if( NOT HAS_GETOPT )
	include_directories ("${GETOPT_INCLUDE}")
endif( NOT HAS_GETOPT )

link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

if( FFTW_FOUND )
  set( FFTW_LIBRARIES ${FFTW_LIBS} )
  set( FAST_BILATERAL "fastbilateral.cpp" )
else( FFTW_FOUND )
  set( FFTW_LIBRARIES )
endif( FFTW_FOUND )

set(TRG pfstmo_durand02)
add_executable(${TRG} ${TRG}.cpp tmo_durand02.cpp bilateral.cpp "${FAST_BILATERAL}" "${GETOPT_OBJECT}")
target_link_libraries(${TRG} pfs ${FFTW_LIBRARIES})
install (TARGETS ${TRG} DESTINATION bin)
install (FILES ${TRG}.1 DESTINATION ${MAN_DIR})
