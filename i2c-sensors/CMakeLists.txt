cmake_minimum_required(VERSION "2.6.0")
PROJECT(i2c-sensors)

SET(VEHICLE_VERSION 0.2+devel)
SET(PROJECT_NAME i2c-sensors)

if(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/windows)
else(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/linux)
endif (WIN32)

find_package(Ice REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${ICE_INCLUDE_DIR})

link_directories(${ICE_LIBRARY_DIR})

set(ICE_SLICE2CPP slice2cpp)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/admin.cpp ${PROJECT_BINARY_DIR}/admin.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice")

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/sensors.cpp ${PROJECT_BINARY_DIR}/sensors.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice")

#                     [DEPENDS [depends...]]
#                     [IMPLICIT_DEPENDS <lang1> depend1 ...]
#                     [WORKING_DIRECTORY dir]

set(CPPFILES
  ${PROJECT_BINARY_DIR}/admin.cpp
  ${PROJECT_BINARY_DIR}/sensors.cpp
  ${PROJECT_BINARY_DIR}/sensors.cpp
  src/from-u-boot/omap24xx_i2c.c
  src/from-u-boot/enable_i2c_clocks.c
  src/UpdaterThread.cpp
  src/IceSvc.cpp
  src/main.cpp)

set(HEADERS
  ${PROJECT_BINARY_DIR}/admin.h
  ${PROJECT_BINARY_DIR}/sensors.h
  ${PROJECT_BINARY_DIR}/sensors.h
  src/UpdaterThread.h
  src/SensorAdminI.h
  src/SensorGroupI.h
  src/IceSvc.h)
              
set(LIBRARIES 
    Ice IceUtil Glacier2
   )

add_executable(i2c-sensors ${CPPFILES} ${HEADERS})

target_link_libraries(i2c-sensors
                      ${LIBRARIES})

install(TARGETS i2c-sensors
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
