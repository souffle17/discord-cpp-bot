cmake_minimum_required(VERSION 3.0.0)
project(DiscordBot VERSION 0.1.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

aux_source_directory("src" src)
add_executable(${PROJECT_NAME} ${src})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
include(FindPackageHandleStandardArgs)

# dependencies
# adapted from https://dpp.dev/buildcmake.html

find_path(DPP_INCLUDE_DIR NAMES dpp/dpp.h HINTS ${DPP_ROOT_DIR})
find_path(OGG_INCLUDE_DIR NAMES ogg/ogg.h HINTS ${OGG_ROOT_DIR})
find_path(OPUS_INCLUDE_DIR NAMES opus/opus.h HINTS ${OPUS_ROOT_DIR})
find_path(OPUSFILE_INCLUDE_DIR NAMES opus/opusfile.h HINTS ${OPUSFILE_ROOT_DIR})
	 
find_library(DPP_LIBRARIES NAMES dpp "libdpp.a" HINTS ${DPP_ROOT_DIR})
find_library(OGG_LIBRARIES NAMES libogg "libogg.so" HINTS ${OGG_ROOT_DIR})
find_library(OPUS_LIBRARIES NAMES libopus0 "libopus.so" HINTS ${OPUS_ROOT_DIR})
find_library(OPUSFILE_LIBRARIES NAMES libopusfile0 "libopusfile.so" HINTS ${OPUSFILE_ROOT_DIR})
 
find_package_handle_standard_args(DPP DEFAULT_MSG DPP_LIBRARIES DPP_INCLUDE_DIR)
find_package_handle_standard_args(OGG DEFAULT_MSG OGG_LIBRARIES OGG_INCLUDE_DIR)
find_package_handle_standard_args(OGG DEFAULT_MSG OGG_LIBRARIES OGG_INCLUDE_DIR)
find_package_handle_standard_args(OGG DEFAULT_MSG OGG_LIBRARIES OGG_INCLUDE_DIR)

target_link_libraries(${PROJECT_NAME} 
    ${DPP_LIBRARIES}
    ${OGG_LIBRARIES}
    ${OPUS_LIBRARIES}
    ${OPUSFILE_LIBRARIES}
)

# include

target_include_directories(${PROJECT_NAME} PRIVATE
    ${DPP_INCLUDE_DIR}
    ${OGG_INCLUDE_DIR}
    ${OPUS_INCLUDE_DIR}
    ${OPUSFILE_INCLUDE_DIR}
    ${OPUSFILE_INCLUDE_DIR}/opus
    src/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)