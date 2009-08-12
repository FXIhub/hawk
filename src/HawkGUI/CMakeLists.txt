project(uwrapc)



INCLUDE_DIRECTORIES(${LOCAL_INCLUDE}/HawkGUI)
INCLUDE_DIRECTORIES(${QWT_INCLUDE_DIR})

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
  ../math_parser.c
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
  imageviewpanel.cpp
  editorworkspace.cpp
  imageeditorview.cpp
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
  ${LOCAL_INCLUDE}/HawkGUI/imageviewpanel.h
  ${LOCAL_INCLUDE}/HawkGUI/editorworkspace.h
  ${LOCAL_INCLUDE}/HawkGUI/imageeditorview.h
  ${LOCAL_INCLUDE}/communicator.h
  ${LOCAL_INCLUDE}/client_decoder.h 
  ${LOCAL_INCLUDE}/decoder.h 
  ${LOCAL_INCLUDE}/sender.h
)


QT4_ADD_RESOURCES(HAWK_QRC_FILES hawk.qrc)

QT_WRAP_CPP(HawkGUI GUI_MOC_FILES ${GUI_MOC_HDRS})

LIST(APPEND GUI_OBJ ${GUI_SRCS} ${GUI_MOC_FILES} ${HAWK_QRC_FILES})

ADD_EXECUTABLE(HawkGUI ${GUI_OBJ})

LIST(APPEND LIBRARY_LIST ${QWT_LIBRARIES})

TARGET_LINK_LIBRARIES(HawkGUI ${LIBRARY_LIST})

#INSTALL(TARGETS HawkGUI 
#	       RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
#	       LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#	       ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

INSTALL(TARGETS HawkGUI 
	       RUNTIME DESTINATION bin
	       LIBRARY DESTINATION lib
	       ARCHIVE DESTINATION lib)


INCLUDE(InstallRequiredSystemLibraries)


SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "My funky project")
SET(CPACK_PACKAGE_VENDOR "Me, myself, and I")
#SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR "1")
SET(CPACK_PACKAGE_VERSION_MINOR "3")
SET(CPACK_PACKAGE_VERSION_PATCH "2")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET(CPACK_PACKAGE_EXECUTABLES "HawkGUI" "Hawk GUI")
#SET(CPACK_PACKAGE_EXECUTABLES "uwrapc" "uwrapc")
SET(CPACK_GENERATOR "TGZ")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "filipe@xray.bmc.uu.se")
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Hawk image reconstruction package GUI")


INCLUDE(CPack)
