cmake_minimum_required(VERSION 3.10)

project(BluepyHelper)

set(BLUEZ_PATH ${CMAKE_SOURCE_DIR}/bluez-5.47)
include_directories(
    ${BLUEZ_PATH}
    ${BLUEZ_PATH}/attrib
    ${BLUEZ_PATH}/lib
    ${BLUEZ_PATH}/src
    ${BLUEZ_PATH}/gdbus
    ${BLUEZ_PATH}/btio
    ${BLUEZ_PATH}/sys
    ${GLIB2_INCLUDE_DIRS}
    ${BLUEZ_PATH}/src/shared
)

set(BLUEZ_SRCS
    ${BLUEZ_PATH}/lib/bluetooth.c
    ${BLUEZ_PATH}/lib/hci.c
    ${BLUEZ_PATH}/lib/sdp.c
    ${BLUEZ_PATH}/lib/uuid.c
    ${BLUEZ_PATH}/attrib/att.c
    ${BLUEZ_PATH}/attrib/gatt.c
    ${BLUEZ_PATH}/attrib/gattrib.c
    ${BLUEZ_PATH}/attrib/utils.c
    ${BLUEZ_PATH}/btio/btio.c
    ${BLUEZ_PATH}/src/log.c
    ${BLUEZ_PATH}/src/shared/mgmt.c
    ${BLUEZ_PATH}/src/shared/crypto.c
    ${BLUEZ_PATH}/src/shared/att.c
    ${BLUEZ_PATH}/src/shared/queue.c
    ${BLUEZ_PATH}/src/shared/util.c
    ${BLUEZ_PATH}/src/shared/io-glib.c
    ${BLUEZ_PATH}/src/shared/timeout-glib.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
# find_package(fmt REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DBLUEPY_DEBUG=1 -DHAVE_CONFIG_H=1 -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DHAVE_CONFIG_H=1 -Os")

add_definitions(-DHAVE_CONFIG_H)
if(DEBUG)
    add_definitions(-DBLUEPY_DEBUG)
endif()

add_library(bluez STATIC  ${BLUEZ_SRCS})
# add_library(bluez SHARED  ${BLUEZ_SRCS})


# set(BLETOOLKIT_SRC blezpp.cc mgmt_wrapper.cpp)
set(BLETOOLKIT_SRC blezpp.cc)
# set(BLETOOLKIT_SRC bluepy-helper.cc blue-dev.c)
# set(BLETOOLKIT_SRC bluepy-helper.cc bluepy-dev.c)

add_library(bluepy SHARED  ${BLUEZ_SRCS} ${BLETOOLKIT_SRC})
add_executable(bletookit ${BLUEZ_SRCS} ${BLETOOLKIT_SRC} bletoolkit.cc )
# target_link_libraries(bletookit PRIVATE ${GLIB2_LIBRARIES} bluez)
target_link_libraries(bletookit ${GLIB2_LIBRARIES} bluez)
