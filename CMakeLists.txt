cmake_minimum_required(VERSION "2.6.0")
PROJECT(vehicle)

SET(VEHICLE_VERSION 0.2+devel)
SET(PROJECT_NAME vehicle)

if(WIN32)
  set(GLIB2_DIR     /usr/src/races/robot/src/ExtLibs/glib-2.0)
  set(GSTREAMER_DIR /usr/src/races/robot/src/ExtLibs/gstreamer-0.10)
  set(LIBXML2_DIR /usr/src/races/robot/src/ExtLibs/libxml2)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/windows)
else(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/linux)
endif (WIN32)

find_package(Glib2 REQUIRED)
find_package(Gstreamer REQUIRED)
find_package(IceE REQUIRED)

# Add C and LD flags needed for Xenomai
EXEC_PROGRAM ("which xeno-config 2>/dev/null" 
              OUTPUT_VARIABLE XENOCONFIG)

EXEC_PROGRAM ("${XENOCONFIG} --skin=native --cflags" 
              OUTPUT_VARIABLE XENO_CFLAGS)
SET (CMAKE_C_FLAGS ${CMAKE_C_FLAGS} ${XENO_CFLAGS})
#SET (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${XENO_CFLAGS})

EXEC_PROGRAM ("${XENOCONFIG} --skin=native --ldflags" 
              OUTPUT_VARIABLE XENO_LDFLAGS)
SET (CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} ${XENO_LDFLAGS})


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
                    ${ICEE_INCLUDE_DIR})

link_directories(${GLIB2_LIBRARIES}
                 ${GSTREAMER_LIBRARIES}
                 ${ICEE_LIBRARY_DIR})

# Disable string control settings for cameras since it causes compilation errors for BeagleBoard
add_definitions(-DDISABLE_STRING_CONTROLS)

if(WIN32)
  add_definitions(-DICEE_STATIC_LIBS)
endif()

#STRING( REGEX REPLACE "\\.ice" .cpp SOURCE_OUTPUT_BASENAME "${SLICE_SOURCE_BASENAME}" )
#STRING( REGEX REPLACE "\\.ice" .h HEADER_OUTPUT_BASENAME "${SLICE_SOURCE_BASENAME}" )

if(WIN32)
  set(ICEE_SLICE2CPPE ${ICEE_HOME}/cpp/bin/slice2cppe)
else(WIN32)
  set(ICEE_SLICE2CPPE slice2cppe)
endif(WIN32)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/comtypes.cpp ${PROJECT_BINARY_DIR}/comtypes.h
                   COMMAND ${ICEE_SLICE2CPPE} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-intrefaces/comtypes.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-intrefaces/comtypes.ice
                   COMMENT "invoking slice2cppe on ${PROJECT_SOURCE_DIR}/remote-intrefaces/comtypes.ice")


add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/vehicleadmin.cpp ${PROJECT_BINARY_DIR}/vehicleadmin.h
                   COMMAND ${ICEE_SLICE2CPPE} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-intrefaces/vehicleadmin.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-intrefaces/vehicleadmin.ice
                   COMMENT "invoking slice2cppe on ${PROJECT_SOURCE_DIR}/remote-intrefaces/vehicleadmin.ice")


add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/vehicle.cpp ${PROJECT_BINARY_DIR}/vehicle.h
                   COMMAND ${ICEE_SLICE2CPPE} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-intrefaces/vehicle.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-intrefaces/vehicle.ice
                   COMMENT "invoking slice2cppe on ${PROJECT_SOURCE_DIR}/slice/vehicle.ice")

#                     [DEPENDS [depends...]]
#                     [IMPLICIT_DEPENDS <lang1> depend1 ...]
#                     [WORKING_DIRECTORY dir]

set(CPPFILES
  ${PROJECT_BINARY_DIR}/comtypes.cpp
  ${PROJECT_BINARY_DIR}/vehicleadmin.cpp
  ${PROJECT_BINARY_DIR}/vehicle.cpp
  src/ActuatorAdminI.cpp
  src/ActuatorI.cpp
  src/VideoSenderThread.cpp
  src/NavSensorThread.cpp
  src/ReadSonarData.cpp
  src/ReadCompassData.cpp
  src/ReadGPSData.cpp
  src/CameraAdminI.cpp
  src/VideoCameraSensorI.cpp
  src/GenericSensorAdminI.cpp
  src/GenericSensorI.cpp
  src/RemoteVehicleI.cpp
  src/MotorControlI_PWM.cpp
  src/xenopwm.c
  src/ServoControl.cpp
  src/Statystic.cpp
  src/AMI_SensorFrameReceiver_nextSensorFrameI.cpp
  src/main.cpp)

set(HEADERS
  ${PROJECT_BINARY_DIR}/comtypes.h
  ${PROJECT_BINARY_DIR}/vehicleadmin.h
  ${PROJECT_BINARY_DIR}/vehicle.h
  src/SensorActuatorIDs.h
  src/AMI_SensorFrameReceiver_nextSensorFrameI.h
  src/ActuatorAdminI.h
  src/ActuatorI.h
  src/CameraAdminI.h
  src/MotorControlI_PWM.h
  src/xenopwm.c
  src/ServoControl.h
  src/RemoteVehicleI.h
  src/Statystic.h
  src/VideoCameraSensorI.h
  src/GenericSensorAdminI.h
  src/GenericSensorI.h
  src/VideoSenderThread.h
  src/NavSensorThread.h
  src/ReadSonarData.h
  src/ReadCompassData.h
  src/ReadGPSData.h)
              
set(LIBRARIES 
    ${GLIB_glib_2_LIBRARY}
    ${GLIB_gobject_2_LIBRARY}
    ${GSTREAMER_gstreamer_LIBRARY}
    ${GSTREAMER_gstapp_LIBRARY})

if(WIN32)
  set(LIBRARIES 
      ${LIBRARIES}
      icee_staticd
      rpcrt4
      Ws2_32)
else(WIN32)
  set(CPPFILES
      ${CPPFILES}
      src/i2c-from-u-boot/omap24xx_i2c.cpp
      src/omap3530-pwm.c
      src/camctlutils.cpp)

  set(HEADERS
      ${HEADERS}
      src/i2c-from-u-boot/i2c.h
      src/i2c-from-u-boot/i2c_interface.h
      src/i2c-from-u-boot/io.h
      src/i2c-from-u-boot/omap24xx_i2c.h
      src/omap3530-pwm.h
      src/camctlutils.h)

  set(LIBRARIES 
      ${LIBRARIES}
      IceE 
      rt
      v4l2
      gps)
endif(WIN32)

add_executable(vehicle ${CPPFILES} ${HEADERS})

target_link_libraries(vehicle
                      ${LIBRARIES})

install(TARGETS vehicle
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
