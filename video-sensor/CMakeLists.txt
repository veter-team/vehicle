cmake_minimum_required(VERSION "2.6.0")
PROJECT(video-sensor)

SET(VEHICLE_VERSION 0.2+devel)
SET(PROJECT_NAME video-sensor)

if(WIN32)
  set(GLIB2_DIR     /usr/src/races/robot/src/ExtLibs/glib-2.0)
  set(GSTREAMER_DIR /usr/src/races/robot/src/ExtLibs/gstreamer-0.10)
  set(LIBXML2_DIR /usr/src/races/robot/src/ExtLibs/libxml2)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake-modules/windows)
else(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake-modules/linux)
endif (WIN32)

find_package(Glib2 REQUIRED)
find_package(Gstreamer REQUIRED)
find_package(Ice REQUIRED)

# conditionnal required packages
INCLUDE (CheckIncludeFiles)
CHECK_INCLUDE_FILES (${GSTREAMER_DIR}/include/gst/gstconfig.h GST_USING_BINARY_REGISTRY)
if (NOT GST_USING_BINARY_REGISTRY)
find_package(LibXml2 REQUIRED)
endif (NOT GST_USING_BINARY_REGISTRY)


include_directories(${GLIB2_INCLUDE_DIR} 
                    ${GSTREAMER_INCLUDE_DIR}
                    ${GSTREAMER_gst_INCLUDE_DIR}
                    ${LIBXML2_INCLUDE_DIR}
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${ICE_INCLUDE_DIR})

link_directories(${GLIB2_LIBRARIES}
                 ${GSTREAMER_LIBRARIES}
                 ${ICE_LIBRARY_DIR})

# Disable string control settings for cameras since it causes compilation errors for BeagleBoard
add_definitions(-DDISABLE_STRING_CONTROLS)

set(ICE_SLICE2CPP slice2cpp)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/admin.cpp ${PROJECT_BINARY_DIR}/admin.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/../remote-intrefaces/admin.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/../remote-intrefaces/admin.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/../remote-intrefaces/admin.ice")

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/sensors.cpp ${PROJECT_BINARY_DIR}/sensors.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/../remote-intrefaces/sensors.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/../remote-intrefaces/sensors.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/../remote-intrefaces/sensors.ice")

#                     [DEPENDS [depends...]]
#                     [IMPLICIT_DEPENDS <lang1> depend1 ...]
#                     [WORKING_DIRECTORY dir]

set(CPPFILES
  ${PROJECT_BINARY_DIR}/admin.cpp
  ${PROJECT_BINARY_DIR}/sensors.cpp
  src/camctlutils.cpp
  src/IceSvc.cpp
  src/UpdaterThread.cpp
  src/SensorAdminI.cpp
  src/SensorGroupI.cpp
  src/main.cpp)

set(HEADERS
  ${PROJECT_BINARY_DIR}/admin.h
  ${PROJECT_BINARY_DIR}/sensors.h
  src/camctlutils.h
  src/UpdaterThread.h
  src/SensorAdminI.h
  src/SensorGroupI.h
  src/IceSvc.h)
              
set(LIBRARIES 
    ${GLIB_glib_2_LIBRARY}
    ${GLIB_gobject_2_LIBRARY}
    ${GSTREAMER_gstreamer_LIBRARY}
    ${GSTREAMER_gstapp_LIBRARY}
    v4l2
    Ice IceUtil Glacier2
   )

add_executable(video-sensor ${CPPFILES} ${HEADERS})

target_link_libraries(video-sensor
                      ${LIBRARIES})

install(TARGETS video-sensor
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
