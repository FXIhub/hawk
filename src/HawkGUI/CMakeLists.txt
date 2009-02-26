project(uwrapc)

INCLUDE_DIRECTORIES(${LOCAL_INCLUDE}/HawkGUI)

set(GUI_SRCS
  ../algorithms.c
  ../configuration.c
  ../log.c
  ../metadata.c
  ../support.c
  ../uwrapc.c
  ../libconfig/grammar.c
  ../libconfig/libconfig.c
  ../libconfig/scanner.c
  ../network_communication.cpp
  ../communicator.cpp
  ../client_decoder.cpp
  ../sender.cpp
  ../decoder.cpp
  ../saddle/minmaxL.c
  ../saddle/minmaxtau.c
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
  optionstree.cpp
  outputwatcher.cpp
  plotdisplay.cpp
  processcontrol.cpp
  processdisplay.cpp
  treedelegate.cpp
)

set(GUI_MOC_HDRS
  ${LOCAL_INCLUDE}/HawkGUI/hawkgui.h 
  ${LOCAL_INCLUDE}/HawkGUI/imagedisplay.h 
  ${LOCAL_INCLUDE}/HawkGUI/imageview.h 
  ${LOCAL_INCLUDE}/HawkGUI/treedelegate.h 
  ${LOCAL_INCLUDE}/HawkGUI/mapeditordialog.h 
  ${LOCAL_INCLUDE}/HawkGUI/optionstree.h 
  ${LOCAL_INCLUDE}/HawkGUI/processdisplay.h 
  ${LOCAL_INCLUDE}/HawkGUI/processcontrol.h 
  ${LOCAL_INCLUDE}/HawkGUI/outputwatcher.h 
  ${LOCAL_INCLUDE}/HawkGUI/imageframe.h 
  ${LOCAL_INCLUDE}/HawkGUI/plotdisplay.h 
  ${LOCAL_INCLUDE}/HawkGUI/logtailer.h 
  ${LOCAL_INCLUDE}/HawkGUI/imageloader.h
  ${LOCAL_INCLUDE}/communicator.h
  ${LOCAL_INCLUDE}/client_decoder.h 
  ${LOCAL_INCLUDE}/decoder.h 
  ${LOCAL_INCLUDE}/sender.h
)



QT_WRAP_CPP(Hawk GUI_MOC_FILES ${GUI_MOC_HDRS})

LIST(APPEND GUI_OBJ ${GUI_SRCS} ${GUI_MOC_FILES})

ADD_EXECUTABLE(hawk ${GUI_OBJ})

LIST(APPEND LIBRARY_LIST qwt)

TARGET_LINK_LIBRARIES(hawk ${LIBRARY_LIST})

INSTALL(TARGETS hawk
	       RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
	       LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	       ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

