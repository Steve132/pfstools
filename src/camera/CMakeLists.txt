include_directories ("${PROJECT_BINARY_DIR}/"
"${PROJECT_SOURCE_DIR}/src/pfs" "${CMAKE_CURRENT_SOURCE_DIR}")

link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

set(TRG pfshdrcalibrate)
add_executable(${TRG} ${TRG}.cpp  responses.cpp robertson02.cpp  mitsunaga99.cpp  mitsunaga99_numerical.cpp nrutil.cpp "${GETOPT_OBJECT}")
			  
target_link_libraries(${TRG} pfs)
install (TARGETS ${TRG} DESTINATION bin)
install (FILES ${TRG}.1 DESTINATION ${MAN_DIR})
