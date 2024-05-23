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

set(LOCAL_SRCS bluepy-helper.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DBLUEPY_DEBUG=1 -DHAVE_CONFIG_H=1 -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DHAVE_CONFIG_H=1 -Os")

add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DBLUEPY_DEBUG)

if(BLUEPY)
    add_definitions(-DBLUEPY_MAIN)
    add_executable(bluepy-helper ${LOCAL_SRCS} ${BLUEZ_SRCS})
    target_link_libraries(bluepy-helper ${GLIB2_LIBRARIES})
else()
    set(BLETOOLKIT_SRC bluepy-helper.c bletoolkit.cpp)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})

    add_executable(bletookit ${BLETOOLKIT_SRC} ${BLUEZ_SRCS})
    target_link_libraries(bletookit ${GLIB2_LIBRARIES})
endif()
