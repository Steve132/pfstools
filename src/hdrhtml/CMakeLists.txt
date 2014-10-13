include_directories ("${PROJECT_BINARY_DIR}/" "${PROJECT_SOURCE_DIR}/src/pfs" ${ImageMagick_INCLUDE_DIRS})
link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

add_executable(pfsouthdrhtml pfsouthdrhtml.cpp hdrhtml.cpp)
target_link_libraries(pfsouthdrhtml pfs ${ImageMagick_LIBRARIES})
install (TARGETS pfsouthdrhtml DESTINATION bin)
install (FILES pfsouthdrhtml.1 DESTINATION ${MAN_DIR})
