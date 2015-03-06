include_directories ("${PROJECT_BINARY_DIR}/"
"${PROJECT_SOURCE_DIR}/src/pfs" "${PROJECT_SOURCE_DIR}/src/tmo/pfstmo")
if( NOT HAS_GETOPT )
	include_directories ("${GETOPT_INCLUDE}")
endif( NOT HAS_GETOPT )
link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

if( FFTW_FOUND AND OPENMP_FOUND)
  set( FFTW_LIBRARIES ${FFTW_LIBS} )
  set( PDE_FFT "pde_fft.cpp" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
  set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_C_FLAGS}" )
else( FFTW_FOUND AND OPENMP_FOUND )
  set( FFTW_LIBRARIES )
endif( FFTW_FOUND AND OPENMP_FOUND )


set(TRG pfstmo_fattal02)
add_executable(${TRG} ${TRG}.cpp tmo_fattal02.cpp pde.cpp ${PDE_FFT} "${GETOPT_OBJECT}")
target_link_libraries(${TRG} pfs ${FFTW_LIBRARIES})
install (TARGETS ${TRG} DESTINATION bin)
install (FILES ${TRG}.1 DESTINATION ${MAN_DIR})
