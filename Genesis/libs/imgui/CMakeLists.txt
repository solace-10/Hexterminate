cmake_minimum_required(VERSION 3.14)
project(imgui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCE_IMGUI CONFIGURE_DEPENDS "src/imgui/*.cpp" "src/imgui/*.h")
file(GLOB SOURCE_IMGUI_STDLIB CONFIGURE_DEPENDS "src/imgui/misc/cpp/*.cpp" "src/imgui/misc/cpp/*.h")
set(SOURCE_FILES ${SOURCE_IMGUI} ${SOURCE_IMGUI_STDLIB})

source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src/imgui FILES ${SOURCE_FILES})

include_directories("src/")
include_directories("src/imgui")

add_library(imgui STATIC ${SOURCE_FILES})
set_target_properties(imgui PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE})

if(WIN32)
    target_compile_options(imgui PUBLIC $<$<CONFIG:Debug>:/MTd> $<$<CONFIG:Release>:/MT>)
endif()
