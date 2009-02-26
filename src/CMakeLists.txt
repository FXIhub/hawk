
PROJECT (uwrapc)


IF(QT4_FOUND)
QT_WRAP_CPP(uwrapc project_moc_files  communicator.h client_decoder.h decoder.h sender.h)
ENDIF(QT4_FOUND)


LIST(APPEND INCLUDE_LIST ${LOCAL_INCLUDE} ${LOCAL_INCLUDE}/saddle ${LOCAL_INCLUDE}/libconfig)

INCLUDE_DIRECTORIES(${INCLUDE_LIST})

SET(UWRAPC_OBJ main.c uwrapc.c configuration.c support.c algorithms.c log.c saddle/minmaxtau.c saddle/minmaxL.c ${project_moc_files})
LIST(APPEND UWRAPC_OBJ communicator.cpp network_communication.cpp libconfig/libconfig.c libconfig/scanner.c libconfig/grammar.c)
LIST(APPEND UWRAPC_OBJ client_decoder.cpp decoder.cpp metadata.c sender.cpp)

ADD_EXECUTABLE(uwrapc ${UWRAPC_OBJ})
ADD_EXECUTABLE(restore_image restore_image.c configuration.c support.c algorithms.c log.c saddle/minmaxtau.c saddle/minmaxL.c libconfig/libconfig.c libconfig/scanner.c libconfig/grammar.c metadata.c )
ADD_EXECUTABLE(process_image process_image.c ${GETOPT})
ADD_EXECUTABLE(post_process post_process.c ${GETOPT})




ADD_SUBDIRECTORY(utils)
IF(QT4_FOUND)
ADD_SUBDIRECTORY(Hawk)
ADD_SUBDIRECTORY(HawkGUI)
ADD_SUBDIRECTORY(Preview)
ADD_SUBDIRECTORY(Look)
ENDIF(QT4_FOUND)
ADD_SUBDIRECTORY(saddle)
ADD_SUBDIRECTORY(tests)


TARGET_LINK_LIBRARIES(uwrapc ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(process_image ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(post_process ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(restore_image ${LIBRARY_LIST})

INSTALL(TARGETS uwrapc process_image post_process restore_image
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
