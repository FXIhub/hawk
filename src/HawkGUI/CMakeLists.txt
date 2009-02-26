project(uwrapc)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

set(GUI_SRCS
  geometrycontrol.cpp
  hawkgui.cpp
  imagecategory.cpp
  imagedisplay.cpp
  imageframe.cpp
  imageitem.cpp
  imageloader.cpp
  imageview.cpp
  logtailer.cpp
  main.cpp
  mapeditordialog.cpp
  network_communication.cpp
  optionstree.cpp
  outputwatcher.cpp
  plotdisplay.cpp
  processcontrol.cpp
  processdisplay.cpp
  treedelegate.cpp
)

set(GUI_MOC_HDRS
  hawkgui.h 
  imagedisplay.h 
  imageview.h 
  treedelegate.h 
  mapeditordialog.h 
  optionstree.h 
  processdisplay.h 
  processcontrol.h 
  outputwatcher.h 
  imageframe.h 
  plotdisplay.h 
  logtailer.h 
  imageloader.h
)



QT_WRAP_CPP(Hawk GUI_MOC_FILES ${GUI_MOC_HDRS})

LIST(APPEND GUI_OBJ ${GUI_SRCS} ${GUI_MOC_FILES})

ADD_EXECUTABLE(hawk ${GUI_OBJ})

LIST(APPEND LIBRARY_LIST ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

TARGET_LINK_LIBRARIES(hawk ${LIBRARY_LIST})

INSTALL(TARGETS hawk
	       RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	       LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	       ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

