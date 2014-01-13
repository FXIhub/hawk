
PROJECT (uwrapc)

LIST(INSERT INCLUDE_LIST 0 ${LOCAL_INCLUDE} ${LOCAL_INCLUDE}/saddle ${LOCAL_INCLUDE}/libconfig  qxt)

 



INCLUDE_DIRECTORIES(${INCLUDE_LIST})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(UWRAPC_OBJ main.c uwrapc.c configuration.c support.c algorithms.c log.c metadata.c)
LIST(APPEND UWRAPC_OBJ io_utils.c saddle/minmaxtau.c saddle/minmaxL.c)
LIST(APPEND UWRAPC_OBJ libconfig/libconfig.c libconfig/scanner.c libconfig/grammar.c)
LIST(APPEND UWRAPC_OBJ output_projection.c)


IF(QT4_FOUND)
QT_WRAP_CPP(uwrapc project_moc_files  ${LOCAL_INCLUDE}/communicator.h ${LOCAL_INCLUDE}/client_decoder.h ${LOCAL_INCLUDE}/decoder.h ${LOCAL_INCLUDE}/sender.h  rpcpeer.h uwrapcpeerthread.h)
QT_WRAP_CPP(uwrapc qxt_moc_files qxt/qxtboundfunction.h qxt/qxtrpcservice.h qxt/qxtrpcservice_p.h qxt/qxtabstractconnectionmanager.h qxt/qxtrpcpeer.h qxt/qxttcpconnectionmanager.h qxt/qxttcpconnectionmanager_p.h)
SET(QXT_OBJS ${CMAKE_CURRENT_SOURCE_DIR}/qxt/qxtrpcservice.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qxt/qxtabstractconnectionmanager.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qxt/qxtmetaobject.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qxt/qxtdatastreamsignalserializer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qxt/qxtrpcpeer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/qxt/qxttcpconnectionmanager.cpp ${qxt_moc_files})
LIST(APPEND UWRAPC_OBJ communicator.cpp network_communication.cpp imagestream.cpp)
LIST(APPEND UWRAPC_OBJ client_decoder.cpp decoder.cpp sender.cpp rpcpeer.cpp uwrapcpeerthread.cpp)
LIST(APPEND UWRAPC_OBJ ${QXT_OBJS})
LIST(APPEND UWRAPC_OBJ ${project_moc_files})
ENDIF(QT4_FOUND)



ADD_EXECUTABLE(uwrapc ${UWRAPC_OBJ})
#ADD_EXECUTABLE(restore_image restore_image.c configuration.c support.c algorithms.c log.c saddle/minmaxtau.c saddle/minmaxL.c libconfig/libconfig.c libconfig/scanner.c libconfig/grammar.c metadata.c io_utils.c network_communication.cpp rpcpeer.cpp ${restore_moc_files} ${QXT_OBJS} uwrapcpeerthread.cpp)

ADD_EXECUTABLE(process_image process_image.c ${GETOPT})
ADD_EXECUTABLE(post_process post_process.c ${GETOPT})
if(A2X)
  SET(HAWK_MAN_PAGES ${CMAKE_BINARY_DIR}/doc/man/hawk.1 ${CMAKE_BINARY_DIR}/doc/man/hawk-convert.1 ${CMAKE_BINARY_DIR}/doc/man/hawk-prepare.1)
  ADD_EXECUTABLE(hawk hawk.c utils/image_to_png.c ${HAWK_MAN_PAGES})
else(A2X)
  ADD_EXECUTABLE(hawk hawk.c utils/image_to_png.c)
endif(A2X)

SET_TARGET_PROPERTIES(hawk PROPERTIES COMPILE_FLAGS -DHAWK_UNIFIED)





ADD_SUBDIRECTORY(utils)
IF(QT4_FOUND)
#ADD_SUBDIRECTORY(Hawk)
if(QWT_FOUND)
  ADD_SUBDIRECTORY(HawkGUI)
endif(QWT_FOUND)
#ADD_SUBDIRECTORY(Preview)

# NOTE: If you want to compile Look just uncomment the line below
#ADD_SUBDIRECTORY(Look)

ENDIF(QT4_FOUND)
#ADD_SUBDIRECTORY(saddle)
ADD_SUBDIRECTORY(tests)
#ADD_SUBDIRECTORY(gtm)
#ADD_SUBDIRECTORY(qxt)


TARGET_LINK_LIBRARIES(uwrapc ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(process_image ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(post_process ${LIBRARY_LIST})
TARGET_LINK_LIBRARIES(hawk ${LIBRARY_LIST})

FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc/man)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/doc/man/hawk.1 COMMAND ${A2X} -f manpage -D ${CMAKE_BINARY_DIR}/doc/man ${CMAKE_SOURCE_DIR}/doc/man/hawk.txt DEPENDS ${CMAKE_SOURCE_DIR}/doc/man/hawk.txt COMMENT "Creating the hawk(1) man page" VERBATIM)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/doc/man/hawk-convert.1 COMMAND ${A2X} -f manpage -D ${CMAKE_BINARY_DIR}/doc/man ${CMAKE_SOURCE_DIR}/doc/man/hawk-convert.txt DEPENDS ${CMAKE_SOURCE_DIR}/doc/man/hawk-convert.txt COMMENT "Creating the hawk-convert(1) man page" VERBATIM)
add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/doc/man/hawk-prepare.1 COMMAND ${A2X} -f manpage -D ${CMAKE_BINARY_DIR}/doc/man ${CMAKE_SOURCE_DIR}/doc/man/hawk-prepare.txt DEPENDS ${CMAKE_SOURCE_DIR}/doc/man/hawk-prepare.txt COMMENT "Creating the hawk-prepare(1) man page" VERBATIM)

INSTALL(TARGETS uwrapc process_image post_process hawk# restore_image
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)
