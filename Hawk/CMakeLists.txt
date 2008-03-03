PROJECT (uwrapc)


QT_WRAP_CPP(Hawk HAWK_MOC_FILES mainwindow.h server.h ../decoder.h server_decoder.h ../communicator.h ../sender.h)

QT4_ADD_RESOURCES(HAWK_QRC_FILES Hawk.qrc)

LIST(APPEND HAWK_OBJ main.cpp mainwindow.cpp server.cpp ${HAWK_MOC_FILES} ${HAWK_QRC_FILES})
LIST(APPEND HAWK_OBJ ../decoder.cpp server_decoder.cpp ../metadata.c ../communicator.cpp ../sender.cpp)

ADD_EXECUTABLE(Hawk ${HAWK_OBJ})

LIST(APPEND LIBRARY_LIST  QtGui)

TARGET_LINK_LIBRARIES(Hawk ${LIBRARY_LIST})

INSTALL(TARGETS Hawk
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
