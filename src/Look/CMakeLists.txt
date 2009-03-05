project(uwrapc)


INCLUDE_DIRECTORIES(${LOCAL_INCLUDE}/Look)
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
	${LOCAL_INCLUDE}/Look/look.h
	${LOCAL_INCLUDE}/Look/mainwindow.h
	${LOCAL_INCLUDE}/Look/imageview.h
	${LOCAL_INCLUDE}/Look/propertiesDialog.h
	${LOCAL_INCLUDE}/Look/cropDialog.h
	${LOCAL_INCLUDE}/Look/backgroundSlider.h
)


MACRO (QT4_WRAP_UI outfiles )
  QT4_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})
  
  FOREACH (it ${ui_files})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfile}.h)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)  
ENDMACRO (QT4_WRAP_UI)

QT4_WRAP_UI(LOOK_UI_FILES supportLevel.ui)

QT_WRAP_CPP(Look LOOK_MOC_FILES ${LOOK_MOC_HDRS})

LIST(APPEND LOOK_OBJ ${LOOK_SRCS} ${LOOK_MOC_FILES})

ADD_EXECUTABLE(Look ${LOOK_OBJ})

LIST(APPEND LIBRARY_LIST ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

TARGET_LINK_LIBRARIES(Look ${LIBRARY_LIST})

INSTALL(TARGETS Look
	       RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	       LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	       ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

