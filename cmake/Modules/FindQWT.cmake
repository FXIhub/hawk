# - Find QWT library
# Find the native QWT includes and library
# This module defines
#  QWT_INCLUDE_DIR, where to find qwt.h, etc.
#  QWT_LIBRARIES, libraries to link against to use QWT.
#  QWT_FOUND, If false, do not try to use QWT.
# also defined, but not for general use are
#  QWT_LIBRARY, where to find the QWT library.

IF(WIN32)
SET(EXTRA_PREFIX c:/MinGW/)
ELSEIF(APPLE)
SET(EXTRA_PREFIX /sw/)
ELSE()
# For some reason they decided to change the location of the include files in 5.2
SET(EXTRA_PREFIX /usr/include/qwt-qt4)
ENDIF()

FIND_PATH(QWT_INCLUDE_DIR qwt.h PATHS ${EXTRA_PREFIX} PATH_SUFFIXES include lib/qwt.framework/Headers)

# They also decided to name it qwt-qt4, which also breaks some stuff
SET(QWT_NAMES ${QWT_NAMES} qwt-qt4 qwt5 qwt)
FIND_LIBRARY(QWT_LIBRARY NAMES ${QWT_NAMES} PATHS ${EXTRA_PREFIX} PATH_SUFFIXES lib)

# handle the QUIETLY and REQUIRED arguments and set QWT_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QWT DEFAULT_MSG QWT_LIBRARY QWT_INCLUDE_DIR)

IF(QWT_FOUND)
  SET( QWT_LIBRARIES ${QWT_LIBRARY} )
ENDIF(QWT_FOUND)
