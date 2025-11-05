cmake_minimum_required(VERSION 3.11)

project(janus)

set(TARGET_NAME ${CMAKE_PROJECT_NAME})

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(FATAL_ERROR "In-source builds not allowed. Please create a separate build directory.")
endif ()

add_library(${TARGET_NAME} INTERFACE)
target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(${TARGET_NAME} INTERFACE cxx_std_17)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default is the Release version" FORCE)
endif ()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
	endif ()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()


set(CMAKE_PREFIX_PATH "D:/OpenCode/hiredis")

find_package(hiredis REQUIRED)
target_link_libraries(janus INTERFACE hiredis::hiredis)

option(ENABLE_TEST "Enable test" OFF)

if (ENABLE_TEST)
	enable_testing()
	add_subdirectory(test)
endif ()

