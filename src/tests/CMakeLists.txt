PROJECT (uwrapc)
if(GSL_FOUND)

INCLUDE_DIRECTORIES(${LOCAL_INCLUDE}/tests)

IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
	ADD_DEFINITIONS(-D_HDF5USEDLL_)
ENDIF(WIN32)


SET(TESTS_OBJ CuTest.c AllTests.c AlgorithmsTests.c GeometryTests.c ../algorithms.c ../configuration.c ../log.c ../saddle/minmaxtau.c ../saddle/minmaxL.c ../libconfig/libconfig.c ../libconfig/scanner.c ../libconfig/grammar.c ../metadata.c ../support.c ../geometry_constraints.c ../io_utils.c ../network_communication.cpp ../uwrapc.c)

IF(QT4_FOUND)
QT_WRAP_CPP(tests qxt_tests_moc_files ../qxt/qxtboundfunction.h ../qxt/qxtrpcservice.h ../qxt/qxtrpcservice_p.h ../qxt/qxtabstractconnectionmanager.h ../qxt/qxtrpcpeer.h ../qxt/qxttcpconnectionmanager.h ../qxt/qxttcpconnectionmanager_p.h  ../rpcpeer.h ../uwrapcpeerthread.h)
SET(QXT_OBJS ../qxt/qxtrpcservice.cpp ../qxt/qxtabstractconnectionmanager.cpp ../qxt/qxtmetaobject.cpp ../qxt/qxtdatastreamsignalserializer.cpp ../qxt/qxtrpcpeer.cpp ../qxt/qxttcpconnectionmanager.cpp)
LIST(APPEND TESTS_OBJ  ../rpcpeer.cpp ../uwrapcpeerthread.cpp ../imagestream.cpp ${QXT_OBJS})
LIST(APPEND TESTS_OBJ ${qxt_tests_moc_files})
ENDIF(QT4_FOUND)


ADD_EXECUTABLE(tests ${TESTS_OBJ})
ADD_EXECUTABLE(geometry_tests CuTest.c GeometryTests.c ../geometry_constraints.c geometry_tests)
INCLUDE_DIRECTORIES(GSL_INCLUDES)
TARGET_LINK_LIBRARIES(tests ${LIBRARY_LIST} ${GSL_LIBRARIES})
TARGET_LINK_LIBRARIES(geometry_tests ${LIBRARY_LIST} ${GSL_LIBRARIES})

endif(GSL_FOUND)





