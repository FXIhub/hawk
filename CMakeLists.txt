cmake_minimum_required(VERSION 2.6)

PROJECT (uwrapc)

SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

ADD_DEFINITIONS(-D H5_USE_16_API)

IF(WIN32)
	ADD_DEFINITIONS(-DQWT_DLL)
	ADD_DEFINITIONS(-D_HDF5USEDLL_)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF()

MACRO (TODAY RESULT)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1" ${RESULT} ${${RESULT}})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1" ${RESULT} ${${RESULT}})
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF (WIN32)
ENDMACRO (TODAY)

TODAY(RESULT)
ADD_DEFINITIONS(-DHAWK_VERSION=${RESULT})

IF(CMAKE_COMPILER_IS_GNUCC)
# Turn on warnings and c99 mode
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall  -W  -std=c99")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -W")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(APPLE 1)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(LINUX 1)
ENDIF()

find_package(TIFF REQUIRED)
find_package(HDF5 REQUIRED)
find_package(FFTW3 REQUIRED)
find_package(SpImage REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(JPEG REQUIRED)
#find_package(SZIP REQUIRED)
find_package(MATH REQUIRED)
find_package(GSL)
#find_package(MATHEVAL)
find_package(QWT)
if(UNIX)
  find_program(A2X NAMES a2x a2x.py a2x.sh DOC "AsciiDoc a2x script, for building man pages")
endif(UNIX)


SET(DMALLOC_USE OFF CACHE BOOL "If ON link to dmalloc library if possible.")
SET(DOUBLE_PRECISION OFF CACHE BOOL "If ON use double precision. Otherwise use single precision")
SET(DEBUG_MEM OFF CACHE BOOL "If ON use memory debugging code.")

SET(USE_QT4 ON CACHE BOOL "If ON use Qt4.")
SET(USE_MPI OFF CACHE BOOL "If ON use MPI for parallelization.")


IF(DMALLOC_USE)
  FIND_LIBRARY (DMALLOC_LIBRARY dmalloc /usr/lib /sw/lib)
  FIND_PATH (DMALLOC_INCLUDE_DIR dmalloc.h PATHS /usr/include /sw/include)	
  IF(DMALLOC_LIBRARY)
    ADD_DEFINITIONS(-D_USE_DMALLOC)
    SET(LINK_TO_DMALLOC 1)
  ENDIF(DMALLOC_LIBRARY)
ENDIF(DMALLOC_USE)


IF(DOUBLE_PRECISION)
	ADD_DEFINITIONS(-D_SP_DOUBLE_PRECISION)
ELSE(DOUBLE_PRECISION)	
	ADD_DEFINITIONS(-D_SP_SINGLE_PRECISION)
ENDIF(DOUBLE_PRECISION)

IF(DEBUG_MEM)
	ADD_DEFINITIONS(-D_SP_DEBUG_MEM)
ENDIF(DEBUG_MEM)



#SET(QT_USE_QTOPENGL 1)
SET(QT_USE_QTNETWORK 1)
IF(USE_QT4)
  INCLUDE(FindQt4)
ENDIF(USE_QT4)

IF(QT_QTCORE_FOUND AND QT_QTNETWORK_FOUND)
	ADD_DEFINITIONS(-DNETWORK_SUPPORT)
ENDIF(QT_QTCORE_FOUND AND QT_QTNETWORK_FOUND)



IF(WIN32)
SET (GETOPT utils/getopt.c)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ELSE(WIN32)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall -std=c99")
ENDIF(WIN32)


# Add rpath to the binaries
# use, i.e. don't skip the full RPATH for the build tree
#SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
#SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# the RPATH to be used when installing
#if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:$ORIGIN/../lib")
#endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")


#if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#  SET(CMAKE_INSTALL_RPATH "@loader_path/../lib")
#endif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


SET(LOCAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

LIST(APPEND INCLUDE_LIST ${SPIMAGE_INCLUDE_DIR} ${TIFF_INCLUDE_DIR} ${HDF5_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR} ${PNG_INCLUDE_DIR})

LIST(APPEND LIBRARY_LIST ${SPIMAGE_LIBRARIES}  ${HDF5_LIBRARIES} ${SZIP_LIBRARIES} ${TIFF_LIBRARIES} ${FFTW_LIBRARIES} ${JPEG_LIBRARIES} ${Z_LIBRARIES} ${MATH_LIBRARIES})
LIST(APPEND LIBRARY_LIST ${SPIMAGE_LIBRARIES})
IF(QT4_FOUND AND QT_USE_FRAMEWORKS)
ELSE(QT4_FOUND AND QT_USE_FRAMEWORKS)
  LIST(APPEND LIBRARY_LIST ${PNG_LIBRARY})
ENDIF(QT4_FOUND AND QT_USE_FRAMEWORKS)

IF(QT4_FOUND)
  INCLUDE(${QT_USE_FILE})
  LIST(APPEND LIBRARY_LIST ${QT_LIBRARIES})
ENDIF(QT4_FOUND)


IF(USE_MPI)
  FIND_PACKAGE(MPI)
  IF(MPI_FOUND)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MPI_COMPILE_FLAGS}")
    LIST(APPEND INCLUDE_LIST ${MPI_INCLUDE_PATH})
    LIST(APPEND LIBRARY_LIST ${MPI_LIBRARIES})
    ADD_DEFINITIONS(-DMPI)
  ENDIF(MPI_FOUND)
ENDIF(USE_MPI)


IF(LINK_TO_DMALLOC)
LIST(APPEND LIBRARY_LIST ${DMALLOC_LIBRARY})
ENDIF(LINK_TO_DMALLOC)

ADD_SUBDIRECTORY(src)

########### Macosx Bundling ###############
IF(APPLE)
  ADD_CUSTOM_TARGET(macosx_bundle perl scripts/macosx_dependencies.pl ${CMAKE_INSTALL_PREFIX}  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "making the macosx bundle." VERBATIM)
#  ADD_CUSTOM_TARGET(macosx_bundle perl scripts/macosx_dependencies.pl ${CMAKE_INSTALL_PREFIX} DEPENDS install  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "making the macosx bundle." VERBATIM)
ENDIF()

########### Linux Bundling ###############

IF(LINUX)
  ADD_CUSTOM_TARGET(linux_bundle perl scripts/linux_dependencies.pl ${CMAKE_INSTALL_PREFIX}   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "making the linux bundle." VERBATIM)
#  ADD_CUSTOM_TARGET(macosx_bundle perl scripts/macosx_dependencies.pl ${CMAKE_INSTALL_PREFIX} DEPENDS install  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "making the macosx bundle." VERBATIM)
ENDIF()


########### Windows Bundling ###############

IF(WIN32)
  ADD_CUSTOM_TARGET(windows_bundle perl scripts/windows_dependencies.pl ${CMAKE_INSTALL_PREFIX}   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "making the windows bundle." VERBATIM)
#  ADD_CUSTOM_TARGET(macosx_bundle perl scripts/macosx_dependencies.pl ${CMAKE_INSTALL_PREFIX} DEPENDS install  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "making the macosx bundle." VERBATIM)
ENDIF()



set(top_level_DOCFILES
  AUTHORS
  COPYING
  Changelog
  Copyright
  PROBLEMS
  README
  )

if(A2X)
#  add_custom_target(documentation ALL ${A2X} -f manpage ${CMAKE_SOURCE_DIR}/doc/man/hawk.txt DEPENDS ${CMAKE_SOURCE_DIR}/doc/man/hawk.txt COMMENT "Creating man pages" VERBATIM)
#add_custom_command(TARGET src/hawk POST_BUILD COMMAND ${A2X} -f manpage -D ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/doc/man/hawk.txt COMMENT "Creating man pages" VERBATIM)

set(man1_pages
  ${CMAKE_BINARY_DIR}/doc/man/hawk.1
  ${CMAKE_BINARY_DIR}/doc/man/hawk-convert.1
  )
install(FILES ${man1_pages} DESTINATION share/man/man1)
endif()

install(FILES ${top_level_DOCFILES} DESTINATION share/doc/hawk)
install(FILES doc/UserManual.pdf DESTINATION share/doc/hawk)
install(DIRECTORY examples DESTINATION share/hawk)
# No CMakeLists.txt there, no point to add it 
#ADD_SUBDIRECTORY(examples)

