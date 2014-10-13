
include_directories ("${PROJECT_BINARY_DIR}/" "${PROJECT_SOURCE_DIR}/src/pfs" ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS})
if( NOT HAS_GETOPT )
	include_directories ("${GETOPT_INCLUDE}")
endif( NOT HAS_GETOPT )

if (OPENMP_FOUND)
#    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_executable(pfsglview pfsglview.cpp picture_io.cpp module.cpp m_histogram.cpp m_status.cpp m_on_screen_display.cpp)

# TODO: Use ${GLUT_LIBRARY} instead.
target_link_libraries(pfsglview ${OPENGL_LIBRARIES} ${GLUT_glut_LIBRARY} pfs)

install (TARGETS pfsglview DESTINATION bin)
install (FILES pfsglview.1 DESTINATION ${MAN_DIR})


