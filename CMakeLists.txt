PROJECT (uwrapc)


FIND_PATH (SPIMAGE_INCLUDE_DIR spimage.h PATHS /usr/include /sw/include)
FIND_PATH (TIFF_INCLUDE_DIR tiff.h PATHS /usr/include /sw/include)
FIND_PATH (HDF5_INCLUDE_DIR hdf5.h PATHS /usr/include /sw/include)
FIND_PATH (FFTW3_INCLUDE_DIR fftw3.h PATHS /usr/include /sw/include)
FIND_PATH (PNG_INCLUDE_DIR png.h PATHS /usr/include /sw/include)
FIND_PATH (QT_INCLUDE_DIR png.h PATHS /usr/include /sw/include)

FIND_LIBRARY (SPIMAGE_LIBRARY spimage /usr/lib /sw/lib)
FIND_LIBRARY (HDF5_LIBRARY hdf5 /usr/lib /sw/lib)
FIND_LIBRARY (TIFF_LIBRARY tiff /usr/lib /sw/lib)
FIND_LIBRARY (PNG_LIBRARY png /usr/lib /sw/lib)
FIND_LIBRARY (MATH_LIBRARY m /usr/lib /sw/lib)
FIND_LIBRARY (C_LIBRARY c /usr/lib /sw/lib)
FIND_LIBRARY (Z_LIBRARY z /usr/lib /sw/lib)
FIND_LIBRARY (JPEG_LIBRARY jpeg /usr/lib /sw/lib)
IF(CMAKE_BUILD_TYPE STREQUAL "debug")
		    FIND_LIBRARY (DMALLOC_LIBRARY dmalloc /usr/lib /sw/lib)
		    FIND_PATH (DMALLOC_INCLUDE_DIR dmalloc.h PATHS /usr/include /sw/include)	
		    IF(DMALLOC_LIBRARY)
			ADD_DEFINITIONS(-D_USE_DMALLOC)
			SET(USE_DMALLOC 1)
		    ENDIF(DMALLOC_LIBRARY)
ENDIF(CMAKE_BUILD_TYPE STREQUAL "debug")


IF(WIN32)
FIND_LIBRARY (FFTW3_LIBRARY fftw3 /usr/lib /sw/lib)
ELSE(WIN32)
FIND_LIBRARY (FFTW3_LIBRARY fftw3 /usr/lib /sw/lib)
FIND_LIBRARY (FFTW3F_LIBRARY fftw3f /usr/lib /sw/lib)
FIND_LIBRARY (FFTW3F_THREADS_LIBRARY fftw3f_threads /usr/lib /sw/lib)	
FIND_LIBRARY (PTHREAD_LIBRARY pthread /usr/lib /sw/lib)	
ENDIF(WIN32)
SET(FFTW_LIBRARY ${FFTW3_LIBRARY} ${FFTW3F_LIBRARY} ${FFTW3F_THREADS_LIBRARY} ${PTHREAD_LIBRARY})


SET(QT_DONT_USE_QTGUI TRUE)
SET(QT_USE_QTNETWORK TRUE)
INCLUDE(FindQt4)

IF(QT_QTCORE_FOUND AND QT_QTNETWORK_FOUND)
	ADD_DEFINITIONS(-DNETWORK_SUPPORT)
ENDIF(QT_QTCORE_FOUND AND QT_QTNETWORK_FOUND)

IF(WIN32)
SET (GETOPT utils/getopt.c)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ELSE(WIN32)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall -std=c99")
ENDIF(WIN32)

LIST(APPEND LIBRARY_LIST ${SPIMAGE_LIBRARY}  ${HDF5_LIBRARY} ${TIFF_LIBRARY} ${PNG_LIBRARY} ${FFTW_LIBRARY} ${JPEG_LIBRARY} ${Z_LIBRARY} ${MATH_LIBRARY} ${C_LIBRARY})

IF(QT4_FOUND)
	INCLUDE_DIRECTORIES(${SPIMAGE_INCLUDE_DIR} ${TIFF_INCLUDE_DIR} ${HDF5_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${QT_INCLUDES} saddle libconfig)
	LIST(APPEND LIBRARY_LIST  QtNetwork QtCore)
ELSE(QT4_FOUND)
	INCLUDE_DIRECTORIES(${SPIMAGE_INCLUDE_DIR} ${TIFF_INCLUDE_DIR} ${HDF5_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR} ${PNG_INCLUDE_DIR}  saddle libconfig)
ENDIF(QT4_FOUND)

IF(QT4_FOUND)
QT_WRAP_CPP(uwrapc project_moc_files communicator.h)
ENDIF(QT4_FOUND)

ADD_EXECUTABLE(uwrapc uwrapc.c configuration.c support.c algorithms.c log.c saddle/minmaxtau.c saddle/minmaxL.c ${project_moc_files} communicator.cpp network_communication.cpp libconfig/libconfig.c libconfig/scanner.c libconfig/grammar.c)
ADD_EXECUTABLE(restore_image restore_image.c configuration.c support.c algorithms.c log.c saddle/minmaxtau.c saddle/minmaxL.c libconfig/libconfig.c libconfig/scanner.c libconfig/grammar.c)
ADD_EXECUTABLE(process_image process_image.c ${GETOPT})
ADD_EXECUTABLE(post_process post_process.c ${GETOPT})



IF(USE_DMALLOC)
LIST(APPEND LIBRARY_LIST ${DMALLOC_LIBRARY})
ENDIF(USE_DMALLOC)

ADD_SUBDIRECTORY(utils)
ADD_SUBDIRECTORY(saddle)


TARGET_LINK_LIBRARIES(uwrapc ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(process_image ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(post_process ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(restore_image ${LIBRARY_LIST})

INSTALL(TARGETS uwrapc process_image post_process restore_image
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
