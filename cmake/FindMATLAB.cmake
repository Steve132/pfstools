# This is a much simplified version of the standard CMake FindMatlab
# script, intended to be used with 'mex' compilation
#=============================================================================

SET(MATLAB_FOUND 0)
SET(MATLAB_ROOT "" CACHE PATH "Matlab root directory")

IF(NOT MATLAB_ROOT)
	IF(WIN32)
		#Find Matlab root. Suppose the Matlab version is between 7.1 and 8.15
		#SET(MATLAB_ROOT "")
		SET(MATLAB_INCLUDE_DIR "")
		SET(_MATLAB_INCLUDE_DIR "")		
		SET(REGISTRY_KEY_TMPL "[HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\TMPL_MAJOR.TMPL_MINOR;MATLABROOT]")		

		# This method will not work with Cygwin
		FOREACH(MATLAB_MAJOR RANGE 7 8)
		FOREACH(MATLAB_MINOR RANGE 15)
			#STRING(REPLACE "TMPL_MINOR" "${MATLAB_MINOR}" REGISTRY_KEY_OUTP ${REGISTRY_KEY_TMPL})
			STRING(REPLACE "TMPL_MINOR" "${MATLAB_MINOR}" REGISTRY_KEY_OUTP ${REGISTRY_KEY_TMPL})
			STRING(REPLACE "TMPL_MAJOR" "${MATLAB_MAJOR}" REGISTRY_KEY_OUTP ${REGISTRY_KEY_OUTP})
			GET_FILENAME_COMPONENT(_MATLAB_ROOT ${REGISTRY_KEY_OUTP} ABSOLUTE)

			IF(NOT ${_MATLAB_ROOT} STREQUAL "/registry")
				SET(MATLAB_ROOT ${_MATLAB_ROOT})
			ENDIF()
		ENDFOREACH()
		ENDFOREACH()
	  
	ELSE( WIN32 )
          # Linux and OSX
          
	       SET(_MATLAB_ROOT_LST
		  $ENV{MATLABDIR}
		  $ENV{MATLAB_DIR}
		  /usr/local/matlab-7sp1/
		  /opt/matlab-7sp1/
		  $ENV{HOME}/matlab-7sp1/
		  $ENV{HOME}/redhat-matlab/
		  /usr/local/MATLAB/R2012b/
		  /usr/local/MATLAB/R2012a/
		  /usr/local/MATLAB/R2011b/
		  /usr/local/MATLAB/R2011a/
		  /usr/local/MATLAB/R2010bSP1/
		  /usr/local/MATLAB/R2010b/
		  /usr/local/MATLAB/R2010a/
		  /usr/local/MATLAB/R2009bSP1/
		  /usr/local/MATLAB/R2009b/
		  /usr/local/MATLAB/R2009a/
		  /usr/local/MATLAB/R2008b/
		  /usr/local/MATLAB/R2008a/
                  /usr/local/matlab9b/
		  /Applications/MATLAB_R2012b.app/
		  /Applications/MATLAB_R2012a.app/
		  /Applications/MATLAB_R2011b.app/
		  /Applications/MATLAB_R2011a.app/
		  /Applications/MATLAB_R2010bSP1.app/
		  /Applications/MATLAB_R2010b.app/
		  /Applications/MATLAB_R2010a.app/
		  /Applications/MATLAB_R2009bSP1.app/
		  /Applications/MATLAB_R2009b.app/
		  /Applications/MATLAB_R2009a.app/
		  /Applications/MATLAB_R2008b.app/
		  /Applications/MATLAB_R2008a.app/
		  )
                
		SET(_MATLAB_ROOT "bin-NOTFOUND")
		FOREACH(LOOP_VAR ${_MATLAB_ROOT_LST})
			FIND_PATH(_MATLAB_ROOT "bin" ${LOOP_VAR} NO_DEFAULT_PATH)
			IF(_MATLAB_ROOT)			
				SET(MATLAB_ROOT ${_MATLAB_ROOT})
				BREAK()
			ENDIF()
		ENDFOREACH()
          
	  
	ENDIF(WIN32)
        
ENDIF(NOT MATLAB_ROOT)
                
if(NOT MATLAB_ROOT)
  message(STATUS "Failed to find matlab root directory. Specify it manually by setting MATLAB_ROOT cmake cache variable.")
else(NOT MATLAB_ROOT)

  find_program(MATLAB_MEX mex PATHS ${MATLAB_ROOT}/bin NO_DEFAULT_PATH NO_SYSTEM_ENVIRONMENT_PATH)

  if(NOT MATLAB_MEX)
    message(STATUS "Failed to find mex matlab compiler.")
  else(NOT MATLAB_MEX)
    set( MATLAB_FOUND ON )
  endif(NOT MATLAB_MEX)
    
endif(NOT MATLAB_ROOT)
