include_directories ("${PROJECT_BINARY_DIR}/"
"${PROJECT_SOURCE_DIR}/src/pfs" "${PROJECT_SOURCE_DIR}/src/tmo/pfstmo" "${GSL_INCLUDE_DIR}")

if( NOT HAS_GETOPT )
	include_directories ("${GETOPT_INCLUDE}")
endif( NOT HAS_GETOPT )

link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

set(TRG pfstmo_mantiuk08)
add_executable(${TRG} ${TRG}.cpp display_adaptive_tmo.cpp display_function.cpp display_size.cpp cqp/cqpminimizer.cpp cqp/initial_point.cpp cqp/mg_pdip.cpp "${GETOPT_OBJECT}")
target_link_libraries(${TRG} pfs ${GSL_LIBRARIES})
install (TARGETS ${TRG} DESTINATION bin)
install (FILES ${TRG}.1 DESTINATION ${MAN_DIR})
