include_directories ("${PROJECT_BINARY_DIR}/"
"${PROJECT_SOURCE_DIR}/src/pfs" "${PROJECT_SOURCE_DIR}/src/tmo/pfstmo")

if( NOT HAS_GETOPT )
	include_directories ("${GETOPT_INCLUDE}")
endif( NOT HAS_GETOPT )

link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

set(TRG pfstmo_ferradans11)
add_executable(${TRG} ${TRG}.cpp Imagen.cpp tmo_ferradans11.cpp "${GETOPT_OBJECT}")
target_link_libraries(${TRG} pfs ${FFTW_LIBS})
install (TARGETS ${TRG} DESTINATION bin)
install (FILES ${TRG}.1 DESTINATION ${MAN_DIR})
