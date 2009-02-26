PROJECT (uwrapc)

INCLUDE_DIRECTORIES(${LOCAL_INCLUDE}/Hawk)

# This it to make the output from the ui files work its magic
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
QT4_ADD_RESOURCES(HAWK_QRC_FILES Hawk.qrc)
QT4_WRAP_UI(HAWK_UI_FILES Hawk.ui workspace.ui)
QT_WRAP_CPP(Hawk HAWK_MOC_FILES ${LOCAL_INCLUDE}/Hawk/mainwindow.h ${LOCAL_INCLUDE}/Hawk/server.h ${LOCAL_INCLUDE}/decoder.h ${LOCAL_INCLUDE}/Hawk/server_decoder.h ${LOCAL_INCLUDE}/communicator.h ${LOCAL_INCLUDE}/sender.h  ${LOCAL_INCLUDE}/Hawk/workspace.h ${LOCAL_INCLUDE}/Hawk/imageViewer.h)

LIST(APPEND HAWK_OBJ main.cpp mainwindow.cpp server.cpp ${HAWK_MOC_FILES} ${HAWK_QRC_FILES})
LIST(APPEND HAWK_OBJ ../decoder.cpp server_decoder.cpp ../metadata.c ../communicator.cpp ../sender.cpp workspace.cpp imageItem.cpp imageViewer.cpp imageBay.cpp)

ADD_EXECUTABLE(Hawk ${HAWK_OBJ})

LIST(APPEND LIBRARY_LIST ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTOPENGL_LIBRARY})

TARGET_LINK_LIBRARIES(Hawk ${LIBRARY_LIST})

INSTALL(TARGETS Hawk
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
