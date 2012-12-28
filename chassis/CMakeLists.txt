cmake_minimum_required(VERSION "2.6.0")
PROJECT(chassis)

SET(VEHICLE_VERSION 0.2+devel)
SET(PROJECT_NAME chassis)

if(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/windows)
else(WIN32)
  list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-modules/linux)
endif (WIN32)

find_package(Ice REQUIRED)

# Add C and LD flags needed for Xenomai
#EXEC_PROGRAM ("PATH=$PATH:${CMAKE_FIND_ROOT_PATH}/usr/bin which xeno-config 2>/dev/null" 
#              OUTPUT_VARIABLE XENOCONFIG)
#EXEC_PROGRAM ("${XENOCONFIG} --skin=native --cflags" 
#              OUTPUT_VARIABLE XENO_CFLAGS)
set(XENO_CFLAGS "-I${CMAKE_FIND_ROOT_PATH}/usr/include/xenomai -D_GNU_SOURCE -D_REENTRANT -D__XENO__")

SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${XENO_CFLAGS}")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${XENO_CFLAGS}")

#EXEC_PROGRAM ("${XENOCONFIG} --skin=native --ldflags" 
#              OUTPUT_VARIABLE XENO_LDFLAGS)
SET(XENO_LDFLAGS "-lnative -lxenomai -lrtdm -lpthread -lrt")

SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${XENO_LDFLAGS}")


include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_BINARY_DIR}
                    ${ICE_INCLUDE_DIR})

link_directories(${ICE_LIBRARY_DIR})

set(ICE_SLICE2CPP slice2cpp)

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/admin.cpp ${PROJECT_BINARY_DIR}/admin.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/admin.ice")

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/actuators.cpp ${PROJECT_BINARY_DIR}/actuators.h
                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/actuators.ice
                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/actuators.ice
                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/actuators.ice")

#add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/sensors.cpp ${PROJECT_BINARY_DIR}/sensors.h
#                   COMMAND ${ICE_SLICE2CPP} -I${ICE_SLICE_DIR} --output-dir ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice
#                   MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice
#                   COMMENT "invoking slice2cpp on ${PROJECT_SOURCE_DIR}/remote-interfaces/sensors.ice")

#                     [DEPENDS [depends...]]
#                     [IMPLICIT_DEPENDS <lang1> depend1 ...]
#                     [WORKING_DIRECTORY dir]

set(CPPFILES
  ${PROJECT_BINARY_DIR}/admin.cpp
  ${PROJECT_BINARY_DIR}/actuators.cpp
#  ${PROJECT_BINARY_DIR}/sensors.cpp
  src/WheelsAdmin.cpp
  src/WheelsGroup.cpp
  src/IceSvc.cpp
  src/main.cpp)

set(HEADERS
  ${PROJECT_BINARY_DIR}/admin.h
  ${PROJECT_BINARY_DIR}/actuators.h
#  ${PROJECT_BINARY_DIR}/sensors.h
  src/MotorController.h
  src/TracksController.h
  src/WheelsAdmin.h
  src/WheelsGroup.h
  src/IceSvc.h)
              
set(LIBRARIES 
    Ice IceUtil Glacier2
   )

add_executable(chassis ${CPPFILES} ${HEADERS})

target_link_libraries(chassis
                      ${LIBRARIES})

install(TARGETS chassis
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
