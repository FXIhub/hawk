PROJECT (uwrapc)


INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
QT4_ADD_RESOURCES(HAWK_QRC_FILES Hawk.qrc)
QT4_WRAP_UI(HAWK_UI_FILES Hawk.ui workspace.ui)
QT_WRAP_CPP(Hawk HAWK_MOC_FILES mainwindow.h server.h ../decoder.h server_decoder.h ../communicator.h ../sender.h  workspace.h ${HAWK_UI_FILES} imageItem.h imageViewer.h imageBay.h)

LIST(APPEND HAWK_OBJ main.cpp mainwindow.cpp server.cpp ${HAWK_MOC_FILES} ${HAWK_QRC_FILES})
LIST(APPEND HAWK_OBJ ../decoder.cpp server_decoder.cpp ../metadata.c ../communicator.cpp ../sender.cpp workspace.cpp imageItem.cpp imageViewer.cpp imageBay.cpp)

ADD_EXECUTABLE(Hawk ${HAWK_OBJ})

LIST(APPEND LIBRARY_LIST ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTOPENGL_LIBRARY})

TARGET_LINK_LIBRARIES(Hawk ${LIBRARY_LIST})

INSTALL(TARGETS Hawk
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
