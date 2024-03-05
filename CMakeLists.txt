cmake_minimum_required(VERSION 3.10.0)

project(frtc CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE "DEBUG")

add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

aux_source_directory(${PROJECT_SOURCE_DIR}/src source)

include_directories(${PROJECT_SOURCE_DIR}/thirdParty/include)

link_directories(${PROJECT_SOURCE_DIR}/thirdParty/lib)

add_library(rtc ${source})

target_link_libraries(rtc
                      ssl
                      crypto)
