cmake_minimum_required(VERSION 3.10.0)

project(frtc CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE "DEBUG")

#add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)

add_compile_options(-fPIC)

aux_source_directory(${PROJECT_SOURCE_DIR}/src source)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/rtp rtp)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/rtcp rtcp)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/base base)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/peer peer)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/video video)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/audio audio)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/media media)

aux_source_directory(${PROJECT_SOURCE_DIR}/src/api api)

include_directories(${PROJECT_SOURCE_DIR}/thirdParty/include)

include_directories(${PROJECT_SOURCE_DIR}/src/)

include_directories(/home/hanly/work/frtc/thirdParty/include)

#link_directories(${PROJECT_SOURCE_DIR}/thirdParty/lib)

link_directories(/home/hanly/work/frtc/thirdParty/lib/arm64)

add_library(rtc 
           ${source} 
           ${rtp} 
           ${rtcp} 
           ${base}
           ${peer}
           ${video}
           ${audio}
           ${media}
           ${api})

#target_link_libraries(rtc srtp2 ssl crypto log)
