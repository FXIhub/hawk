# - Find QWT library
# Find the native QWT includes and library
# This module defines
#  QWT_INCLUDE_DIR, where to find hdf5.h, etc.
#  QWT_LIBRARIES, libraries to link against to use QWT.
#  QWT_FOUND, If false, do not try to use QWT.
# also defined, but not for general use are
#  QWT_LIBRARY, where to find the QWT library.

FIND_PATH(QWT_INCLUDE_DIR qwt.h)

SET(QWT_NAMES ${QWT_NAMES} qwt qwt5)
FIND_LIBRARY(QWT_LIBRARY NAMES ${QWT_NAMES} )

# handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QWT DEFAULT_MSG QWT_LIBRARY QWT_INCLUDE_DIR)

IF(QWT_FOUND)
  SET( QWT_LIBRARIES ${QWT_LIBRARY} )
ENDIF(QWT_FOUND)
