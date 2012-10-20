cmake_minimum_required(VERSION "2.6.0")
PROJECT(vehicle)

SET(VEHICLE_VERSION 0.8+devel)
SET(PROJECT_NAME vehicle)

add_definitions(-D__GLIBC__)

add_subdirectory(chassis)
add_subdirectory(i2c-sensors)
add_subdirectory(servo)
add_subdirectory(video-sensor)

if(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/windows)
else(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/linux)
endif (WIN32)

#install(TARGETS vehicle
#  RUNTIME DESTINATION bin
#  LIBRARY DESTINATION lib
#  ARCHIVE DESTINATION lib
#)
