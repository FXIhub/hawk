# - Find HDF5 library
# Find the native HDF5 includes and library
# This module defines
#  HDF5_INCLUDE_DIR, where to find hdf5.h, etc.
#  HDF5_LIBRARIES, libraries to link against to use HDF5.
#  HDF5_FOUND, If false, do not try to use HDF5.
# also defined, but not for general use are
#  HDF5_LIBRARY, where to find the HDF5 library.

IF(WIN32)
SET(INCLUDE_PATHS c:/MinGW/)
SET(LIB_PATHS c:/MinGW/)
ELSEIF(APPLE)
SET(INCLUDE_PATHS /sw/)
SET(LIB_PATHS /sw/)
ELSE()
SET(INCLUDE_PATHS /usr/local/)
SET(LIB_PATHS /usr/local/)
ENDIF()

FIND_PATH(HDF5_INCLUDE_DIR hdf5.h PATHS ${INCLUDE_PATHS} PATH_SUFFIXES include)
FIND_LIBRARY(HDF5_LIBRARY NAMES ${HDF5_NAMES} PATHS ${LIB_PATHS} PATH_SUFFIXES lib)

# handle the QUIETLY and REQUIRED arguments and set HDF5_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HDF5 DEFAULT_MSG HDF5_LIBRARY HDF5_INCLUDE_DIR)

IF(HDF5_FOUND)
  SET( HDF5_LIBRARIES ${HDF5_LIBRARY} )
ENDIF(HDF5_FOUND)
