PROJECT (uwrapc)


FIND_PATH (SPIMAGE_INCLUDE_DIR spimage.h PATHS /usr/include /sw/include)
FIND_PATH (LIBCONFIG_INCLUDE_DIR libconfig.h PATHS /usr/include /sw/include)
FIND_PATH (TIFF_INCLUDE_DIR tiff.h PATHS /usr/include /sw/include)
FIND_PATH (HDF5_INCLUDE_DIR hdf5.h PATHS /usr/include /sw/include)
FIND_PATH (FFTW3_INCLUDE_DIR fftw3.h PATHS /usr/include /sw/include)
FIND_PATH (PNG_INCLUDE_DIR png.h PATHS /usr/include /sw/include)

FIND_LIBRARY (SPIMAGE_LIBRARY spimage /usr/lib /sw/lib)
FIND_LIBRARY (LIBCONFIG_LIBRARY config /usr/lib /sw/lib)
FIND_LIBRARY (HDF5_LIBRARY hdf5 /usr/lib /sw/lib)
FIND_LIBRARY (TIFF_LIBRARY tiff /usr/lib /sw/lib)
FIND_LIBRARY (PNG_LIBRARY png /usr/lib /sw/lib)
FIND_LIBRARY (MATH_LIBRARY m /usr/lib /sw/lib)
FIND_LIBRARY (C_LIBRARY c /usr/lib /sw/lib)
FIND_LIBRARY (Z_LIBRARY z /usr/lib /sw/lib)
FIND_LIBRARY (JPEG_LIBRARY jpeg /usr/lib /sw/lib)

IF(WIN32)
FIND_LIBRARY (FFTW3_LIBRARY fftw3 /usr/lib /sw/lib)
ELSE(WIN32)
FIND_LIBRARY (FFTW3_LIBRARY fftw3 /usr/lib /sw/lib)
FIND_LIBRARY (FFTW3F_LIBRARY fftw3f /usr/lib /sw/lib)
FIND_LIBRARY (FFTW3F_THREADS_LIBRARY fftw3f_threads /usr/lib /sw/lib)	
FIND_LIBRARY (PTHREAD_LIBRARY pthread /usr/lib /sw/lib)	
ENDIF(WIN32)
SET(FFTW_LIBRARY ${FFTW3_LIBRARY} ${FFTW3F_LIBRARY} ${FFTW3F_THREADS_LIBRARY} ${PTHREAD_LIBRARY})


IF(WIN32)
SET (GETOPT utils/getopt.c)
ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ELSE(WIN32)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall -std=c99")
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${SPIMAGE_INCLUDE_DIR} ${LIBCONFIG_INCLUDE_DIR} ${TIFF_INCLUDE_DIR} ${HDF5_INCLUDE_DIR} ${FFTW3_INCLUDE_DIR} ${PNG_INCLUDE_DIR} saddle)
ADD_EXECUTABLE(uwrapc uwrapc.c configuration.c support.c algorithms.c log.c saddle/minmaxtau.c saddle/minmaxL.c)
ADD_EXECUTABLE(restore_image restore_image.c configuration.c support.c algorithms.c log.c saddle/minmaxtau.c saddle/minmaxL.c)
ADD_EXECUTABLE(process_image process_image.c ${GETOPT})
ADD_EXECUTABLE(post_process post_process.c ${GETOPT})


SET(LIBRARY_LIST ${SPIMAGE_LIBRARY} ${LIBCONFIG_LIBRARY} ${HDF5_LIBRARY} ${TIFF_LIBRARY} ${PNG_LIBRARY} ${FFTW_LIBRARY} ${JPEG_LIBRARY} ${Z_LIBRARY} ${MATH_LIBRARY} ${C_LIBRARY})

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
