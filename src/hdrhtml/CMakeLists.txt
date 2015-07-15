include_directories ("${PROJECT_BINARY_DIR}/" "${PROJECT_SOURCE_DIR}/src/pfs" ${ImageMagick_INCLUDE_DIRS})
link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

add_executable(pfsouthdrhtml pfsouthdrhtml.cpp hdrhtml.cpp)
target_link_libraries(pfsouthdrhtml pfs ${ImageMagick_LIBRARIES})
set_target_properties(pfsouthdrhtml PROPERTIES COMPILE_FLAGS "-DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16")

install (TARGETS pfsouthdrhtml DESTINATION bin)
install (FILES pfsouthdrhtml.1 DESTINATION ${MAN_DIR})

set(DATAFILES hdrhtml_c_b2.csv hdrhtml_t_b2.csv hdrhtml_c_b3.csv hdrhtml_t_b3.csv hdrhtml_c_b4.csv hdrhtml_t_b4.csv hdrhtml_c_b5.csv hdrhtml_t_b5.csv)
                  
foreach(SRC ${DATAFILES}) 
	install (FILES ${SRC} DESTINATION ${PKG_DATADIR})
endforeach(SRC)

install( DIRECTORY "hdrhtml_default_templ" DESTINATION ${PKG_DATADIR} )
install( DIRECTORY "hdrhtml_hdrlabs_templ" DESTINATION ${PKG_DATADIR} )

