# - Find FFTW
# Find the native FFTW includes and library
#
#  FFTW_INCLUDES    - where to find fftw3.h
#  FFTW_LIBRARIES   - List of libraries when using FFTW.
#  FFTW_FOUND       - True if FFTW found.

if (FFTW_INCLUDES)
  # Already in cache, be silent
  set (FFTW_FIND_QUIETLY TRUE)
endif (FFTW_INCLUDES)

find_path (FFTW_INCLUDES fftw3.h)

find_library (FFTW3_LIB NAMES fftw3)

find_library (FFTW3F_LIB NAMES fftw3f)

find_library (FFTW3F_THREADS_LIB NAMES fftw3_threads)

#set (FFTW_LIBRARIES ${FFTW3_LIB} ${FFTW3F_LIB})

# handle the QUIETLY and REQUIRED arguments and set FFTW_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (FFTW DEFAULT_MSG FFTW3_LIB FFTW3F_LIB FFTW3F_THREADS_LIB FFTW_INCLUDES)

SET(FFTW_LIBS ${FFTW3_LIB} ${FFTW3F_LIB} ${FFTW3F_THREADS_LIB})

mark_as_advanced (FFTW_LIBRARIES FFTW_INCLUDES)
