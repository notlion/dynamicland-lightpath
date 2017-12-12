cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(LightpathSim)

get_filename_component(CINDER_PATH "~/code/frameworks/cinder-master" ABSOLUTE)
get_filename_component(APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

include("${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake")

file(GLOB ASIO_SOURCES ${CINDER_PATH}/blocks/Cinder-Asio/src/*.cpp)

ci_make_app(
	APP_NAME    "LightpathSim"

	INCLUDES		${APP_PATH}/include
							${CINDER_PATH}/blocks/Cinder-Asio/src

	SOURCES     ${APP_PATH}/src/LightpathSimApp.cpp
							${APP_PATH}/src/Fx.cpp
							${APP_PATH}/src/OpcClient.cpp
							${ASIO_SOURCES}

	RESOURCES   ${APP_PATH}/res/led_splat_0.png

	CINDER_PATH ${CINDER_PATH}
)
