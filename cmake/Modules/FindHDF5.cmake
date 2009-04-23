# - Find HDF5 library
# Find the native HDF5 includes and library
# This module defines
#  HDF5_INCLUDE_DIR, where to find hdf5.h, etc.
#  HDF5_LIBRARIES, libraries to link against to use HDF5.
#  HDF5_FOUND, If false, do not try to use HDF5.
# also defined, but not for general use are
#  HDF5_LIBRARY, where to find the HDF5 library.
IF(WIN32)
SET(EXTRA_PREFIX c:/MinGW/)
ELSEIF(APPLE)
SET(EXTRA_PREFIX /sw/)
ELSE()
ENDIF()

SET(HDF5_NAMES ${HDF5_NAMES} hdf5 libhdf5)

FIND_PATH(HDF5_INCLUDE_DIR hdf5.h PATHS ${EXTRA_PREFIX} PATH_SUFFIXES include)
FIND_LIBRARY(HDF5_LIBRARY NAMES ${HDF5_NAMES} PATHS ${EXTRA_PREFIX} PATH_SUFFIXES lib)

# handle the QUIETLY and REQUIRED arguments and set HDF5_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HDF5 DEFAULT_MSG HDF5_LIBRARY HDF5_INCLUDE_DIR)

IF(HDF5_FOUND)
  SET( HDF5_LIBRARIES ${HDF5_LIBRARY} )
ENDIF(HDF5_FOUND)
