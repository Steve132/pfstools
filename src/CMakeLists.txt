if( NOT HAS_GETOPT )
  add_subdirectory (getopt)
endif( NOT HAS_GETOPT )

add_subdirectory (pfs) 
add_subdirectory (fileformat) 
add_subdirectory (filter)

if( ImageMagick_FOUND )
    add_subdirectory (hdrhtml)
endif( ImageMagick_FOUND )

if( QT_FOUND )
    add_subdirectory (pfsview)
endif( QT_FOUND )

if( MATLAB_FOUND )
    add_subdirectory (matlab)
endif( MATLAB_FOUND )

if( OPENGL_FOUND AND GLUT_FOUND )
    add_subdirectory (pfsglview)
endif( OPENGL_FOUND AND GLUT_FOUND )

if( WITH_HDRVC )
	add_subdirectory (hdrvc)
endif( WITH_HDRVC )

if( MKOCTFILE )
	add_subdirectory (octave)
endif( MKOCTFILE )

add_subdirectory (tmo)
add_subdirectory (camera)
