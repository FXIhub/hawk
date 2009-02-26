# - Find SPIMAGE library
# Find the native SPIMAGE includes and library
# This module defines
#  SPIMAGE_INCLUDE_DIR, where to find hdf5.h, etc.
#  SPIMAGE_LIBRARIES, libraries to link against to use SPIMAGE.
#  SPIMAGE_FOUND, If false, do not try to use SPIMAGE.
# also defined, but not for general use are
#  SPIMAGE_LIBRARY, where to find the SPIMAGE library.

FIND_PATH(MATHEVAL_INCLUDE_DIR matheval.h)

SET(MATHEVAL_NAMES ${MATHEVAL_NAMES} matheval)
FIND_LIBRARY(MATHEVAL_LIBRARY NAMES ${MATHEVAL_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set MATHEVAL_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MATHEVAL DEFAULT_MSG MATHEVAL_LIBRARY MATHEVAL_INCLUDE_DIR)

IF(MATHEVAL_FOUND)
  SET( MATHEVAL_LIBRARIES ${MATHEVAL_LIBRARY} )
ENDIF(MATHEVAL_FOUND)
