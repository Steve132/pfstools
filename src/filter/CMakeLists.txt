include_directories ("${PROJECT_BINARY_DIR}/" "${PROJECT_SOURCE_DIR}/src/pfs")
if( NOT HAS_GETOPT )
	include_directories ("${GETOPT_INCLUDE}")
endif( NOT HAS_GETOPT )
link_directories("${PROJECT_SOURCE_DIR}/src/pfs")

set(PFS_FILT pfsgamma pfsclamp pfstag pfssize pfsextractchannels
pfspanoramic pfsrotate pfsflip pfscut pfspad pfscat pfsabsolute pfsretime
pfscolortransform) 

foreach(TRG ${PFS_FILT}) 
    add_executable(${TRG} ${TRG}.cpp "${GETOPT_OBJECT}")
    target_link_libraries(${TRG} pfs)
    install (TARGETS ${TRG} DESTINATION bin)
    install (FILES ${TRG}.1 DESTINATION ${MAN_DIR})
 endforeach(TRG) 

add_executable(pfsdisplayfunction pfsdisplayfunction.cpp display_function.cpp display_function.h "${GETOPT_OBJECT}")
target_link_libraries(pfsdisplayfunction pfs)
install (TARGETS pfsdisplayfunction DESTINATION bin)
install (FILES pfsdisplayfunction.1 DESTINATION ${MAN_DIR})

