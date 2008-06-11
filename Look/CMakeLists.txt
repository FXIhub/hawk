project(uwrapc)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

set(LOOK_SRCS
	main.cpp
	look.cpp
	mainwindow.cpp
	imageview.cpp
	propertiesDialog.cpp
	cropDialog.cpp
	backgroundSlider.cpp
)

set(LOOK_MOC_HDRS
	look.h
	mainwindow.h
	imageview.h
	propertiesDialog.h
	cropDialog.h
	backgroundSlider.h
)

QT_WRAP_CPP(Look LOOK_MOC_FILES ${LOOK_MOC_HDRS})

LIST(APPEND LOOK_OBJ ${LOOK_SRCS} ${LOOK_MOC_FILES})

ADD_EXECUTABLE(Look ${LOOK_OBJ})

LIST(APPEND LIBRARY_LIST ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

TARGET_LINK_LIBRARIES(Look ${LIBRARY_LIST})

INSTALL(TARGETS Look
	       RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	       LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	       ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

