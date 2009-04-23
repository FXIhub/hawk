# - Find SPIMAGE library
# Find the native SPIMAGE includes and library
# This module defines
#  SPIMAGE_INCLUDE_DIR, where to find hdf5.h, etc.
#  SPIMAGE_LIBRARIES, libraries to link against to use SPIMAGE.
#  SPIMAGE_FOUND, If false, do not try to use SPIMAGE.
# also defined, but not for general use are
#  SPIMAGE_LIBRARY, where to find the SPIMAGE library.

IF(WIN32)
SET(EXTRA_PREFIX c:/MinGW/)
ELSEIF(APPLE)
SET(EXTRA_PREFIX /sw/)
ELSE()
ENDIF()

FIND_PATH(SPIMAGE_INCLUDE_DIR spimage.h PATHS ${EXTRA_PREFIX} PATH_SUFFIXES include)

SET(SPIMAGE_NAMES ${SPIMAGE_NAMES} spimage)
FIND_LIBRARY(SPIMAGE_LIBRARY NAMES ${SPIMAGE_NAMES} PATHS ${EXTRA_PREFIX} PATH_SUFFIXES lib)

# handle the QUIETLY and REQUIRED arguments and set SPIMAGE_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SPIMAGE DEFAULT_MSG SPIMAGE_LIBRARY SPIMAGE_INCLUDE_DIR)

IF(SPIMAGE_FOUND)
  SET( SPIMAGE_LIBRARIES ${SPIMAGE_LIBRARY} )
ENDIF(SPIMAGE_FOUND)
