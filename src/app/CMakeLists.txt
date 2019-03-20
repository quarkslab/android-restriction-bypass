cmake_minimum_required(VERSION 3.6)
set(ANDROID_ALLOW_UNDEFINED_SYMBOLS 0)
set(CMAKE_BUILD_TYPE Release) # Yes for release!

set(TARGET_ANDROID_VERSION "9.0.0_r9")

STRING(REGEX MATCHALL "([0-9]+)" VERSION_STRING "${TARGET_ANDROID_VERSION}")

list(GET VERSION_STRING 0 ANDROID_VERSION_MAJOR)
list(GET VERSION_STRING 1 ANDROID_VERSION_MINOR)
list(GET VERSION_STRING 2 ANDROID_VERSION_PATCH)
list(GET VERSION_STRING 3 ANDROID_VERSION_REVISION)

set(ANDROID_VERSION "${ANDROID_VERSION_MAJOR}.${ANDROID_VERSION_MINOR}.${ANDROID_VERSION_PATCH}")
set(GIT_TAG "android-${ANDROID_VERSION_MAJOR}.${ANDROID_VERSION_MINOR}.${ANDROID_VERSION_PATCH}_r${ANDROID_VERSION_REVISION}")
message(STATUS "Using Android: ${GIT_TAG}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(fake-aosp)
include(lief)

add_library(hiddenapi-bypass SHARED src/main/cpp/hidden-api-bypass.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
# When CMake from NDK will be upgraded
#set_target_properties(hiddenapi-bypass PROPERTIES CXX_STANDARD              17)
#set_target_properties(hiddenapi-bypass PROPERTIES CXX_STANDARD_REQUIRED     ON)
#set_target_properties(hiddenapi-bypass PROPERTIES POSITION_INDEPENDENT_CODE ON)

find_library(log-lib log)

target_link_libraries(hiddenapi-bypass ${log-lib} liblief)
target_link_libraries(hiddenapi-bypass libbionic-fake -ldl)
target_link_libraries(hiddenapi-bypass libart-fake)
