# Main variables:
# OPENNI_LIBRARY and OPENNI_INCLUDES to link OpenCV modules with OpenNI
# HAVE_OPENNI for conditional compilation OpenCV with/without OpenNI

set(OPENNI_LIB_DESCR "Path to the directory of OpenNI libraries" CACHE INTERNAL "Description" )
set(OPENNI_INCLUDE_DESCR "Path to the directory of OpenNI includes" CACHE INTERNAL "Description" )
set(OPENNI_PRIME_SENSOR_MODULE_BIN_DESCR "Path to the directory of PrimeSensor Module binaries" CACHE INTERNAL "Description" )

if(NOT "${OPENNI_LIB_DIR}" STREQUAL "${OPENNI_LIB_DIR_INTERNAL}")
    unset(OPENNI_LIBRARY CACHE)
endif()
	
if(NOT "${OPENNI_INCLUDE_DIR}" STREQUAL "${OPENNI_INCLUDE_DIR_INTERNAL}")
    unset(OPENNI_INCLUDES CACHE)
endif()
	
if(NOT "${OPENNI_PRIME_SENSOR_MODULE_BIN_DIR}" STREQUAL "${OPENNI_PRIME_SENSOR_MODULE_BIN_DIR}")
    unset(OPENNI_PRIME_SENSOR_MODULE CACHE)
endif()
	
if(NOT OPENNI_LIB_DIR OR OPENNI_LIB_DIR STREQUAL "")
    if(WIN32)
        set(OPENNI_LIB_DIR "c:/Program Files/OpenNI/Lib" CACHE PATH ${OPENNI_LIB_DESCR})
    elseif(UNIX OR APPLE)
        set(OPENNI_LIB_DIR "/usr/lib" CACHE PATH ${OPENNI_LIB_DESCR})
    endif()
endif()
	
if(NOT OPENNI_INCLUDE_DIR OR OPENNI_INCLUDE_DIR STREQUAL "")
    if(WIN32)
        set(OPENNI_INCLUDE_DIR "c:/Program Files/OpenNI/Include" CACHE PATH ${OPENNI_INCLUDE_DESCR})
    elseif(UNIX OR APPLE)
        set(OPENNI_INCLUDE_DIR "/usr/include/ni" CACHE PATH ${OPENNI_INCLUDE_DESCR})
    endif()
endif()
	
if(NOT OPENNI_PRIME_SENSOR_MODULE_BIN_DIR OR OPENNI_PRIME_SENSOR_MODULE_BIN_DIR STREQUAL "")
    if(WIN32)
        set(OPENNI_PRIME_SENSOR_MODULE_BIN_DIR "c:/Program Files/Prime Sense/Sensor/Bin" CACHE PATH ${OPENNI_PRIME_SENSOR_MODULE_BIN_DESCR})
    elseif(UNIX OR APPLE)
        set(OPENNI_PRIME_SENSOR_MODULE_BIN_DIR "/usr/lib" CACHE PATH ${OPENNI_PRIME_SENSOR_MODULE_BIN_DESCR})
    endif()
endif()

find_library(OPENNI_LIBRARY "OpenNI" PATHS ${OPENNI_LIB_DIR} DOC "OpenNI library" NO_DEFAULT_PATH)
find_path(OPENNI_INCLUDES "XnCppWrapper.h" PATHS ${OPENNI_INCLUDE_DIR} DOC "OpenNI c++ interface header" NO_DEFAULT_PATH)

if(OPENNI_LIBRARY AND OPENNI_INCLUDES)
    set(HAVE_OPENNI TRUE)
    # the check: are PrimeSensor Modules for OpenNI installed?
    if(WIN32)
        find_file(OPENNI_PRIME_SENSOR_MODULE "XnCore.dll" PATHS ${OPENNI_PRIME_SENSOR_MODULE_BIN_DIR} DOC "Core library of PrimeSensor Modules for OpenNI" NO_DEFAULT_PATH)
    elseif(UNIX OR APPLE)
        find_library(OPENNI_PRIME_SENSOR_MODULE "XnCore" PATHS ${OPENNI_PRIME_SENSOR_MODULE_BIN_DIR} DOC "Core library of PrimeSensor Modules for OpenNI" NO_DEFAULT_PATH)
    endif()
	
    if(OPENNI_PRIME_SENSOR_MODULE)
        set(HAVE_OPENNI_PRIME_SENSOR_MODULE TRUE)
     endif()
endif() #if(OPENNI_LIBRARY AND OPENNI_INCLUDES)

if(OPENNI_LIBRARY)
    set(OPENNI_LIB_DIR_INTERNAL "${OPENNI_LIB_DIR}" CACHE INTERNAL "This is the value of the last time OPENNI_LIB_DIR was set successfully." FORCE)
else()
    set(OPENNI_LIB_DIR "${OPENNI_LIB_DIR}-NOTFOUND or does not have OpenNI libraries" CACHE PATH ${OPENNI_LIB_DESCR} FORCE)
    message( WARNING, " OpenNI library directory (set by OPENNI_LIB_DIR variable) is not found or does not have OpenNI libraries." )
endif()

if(OPENNI_INCLUDES)
    set(OPENNI_INCLUDE_DIR_INTERNAL "${OPENNI_INCLUDE_DIR}" CACHE INTERNAL "This is the value of the last time OPENNI_INCLUDE_DIR was set successfully." FORCE)
else()
    set(OPENNI_INCLUDE_DIR "${OPENNI_INCLUDE_DIR}-NOTFOUND or does not have OpenNI includes" CACHE PATH ${OPENNI_INCLUDE_DESCR} FORCE)
    message( WARNING, " OpenNI include directory (set by OPENNI_INCLUDE_DIR variable) is not found or does not have OpenNI includes." )
endif()

if(OPENNI_PRIME_SENSOR_MODULE)
    set(OPENNI_PRIME_SENSOR_MODULE_BIN_DIR_INTERNAL "${OPENNI_PRIME_SENSOR_MODULE_BIN_DIR}" CACHE INTERNAL "This is the value of the last time OPENNI_PRIME_SENSOR_MODULE_BIN_DIR was set successfully." FORCE)
else()
    set(OPENNI_PRIME_SENSOR_MODULE_BIN_DIR "${OPENNI_PRIME_SENSOR_MODULE_BIN_DIR}-NOTFOUND or does not have PrimeSensor Module binaries" CACHE PATH ${OPENNI_PRIME_SENSOR_MODULE_BIN_DESCR} FORCE)
    message( WARNING, " PrimeSensor Module binaries directory (set by OPENNI_PRIME_SENSOR_MODULE_BIN_DIR variable) is not found or does not have PrimeSensor Module binaries." )
endif()

mark_as_advanced(FORCE OPENNI_PRIME_SENSOR_MODULE)
mark_as_advanced(FORCE OPENNI_LIBRARY)
mark_as_advanced(FORCE OPENNI_INCLUDES)
