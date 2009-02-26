# - Find MATH library
# Find the native MATH includes and library
# This module defines
#  MATH_INCLUDE_DIR, where to find hdf5.h, etc.
#  MATH_LIBRARIES, libraries to link against to use MATH.
#  MATH_FOUND, If false, do not try to use MATH.
# also defined, but not for general use are
#  MATH_LIBRARY, where to find the MATH library.

FIND_PATH(MATH_INCLUDE_DIR math.h)

SET(MATH_NAMES ${MATH_NAMES} m)
FIND_LIBRARY(MATH_LIBRARY NAMES ${MATH_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set MATH_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MATH DEFAULT_MSG MATH_LIBRARY MATH_INCLUDE_DIR)

IF(MATH_FOUND)
  SET( MATH_LIBRARIES ${MATH_LIBRARY} )
ENDIF(MATH_FOUND)
