PROJECT (uwrapc)


IF(WIN32)
	ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
	ADD_DEFINITIONS(-D_HDF5USEDLL_)
ENDIF(WIN32)


ADD_EXECUTABLE(tests ${OBJECTS} CuTest.c AllTests.c AlgorithmsTests.c ../algorithms.c ../configuration.c ../log.c ../saddle/minmaxtau.c ../saddle/minmaxL.c ../libconfig/libconfig.c ../libconfig/scanner.c ../libconfig/grammar.c ../metadata.c ../support.c)

TARGET_LINK_LIBRARIES(tests ${LIBRARY_LIST})





