cmake_minimum_required(VERSION 3.15)
set(PROJECT_NAME "soloud")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "include/*.h" "src/audiosource/*/*.cpp" "src/audiosource/*/*.c" "src/backend/*/*.cpp" "src/core/*.cpp" "src/filter/*.cpp")

include_directories(
    "include/"
    ${SDL2_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib/${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE})

target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_SDL2_STATIC)

if(WIN32)
    target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:/MTd> $<$<CONFIG:Release>:/MT>)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE WITH_ALSA)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:
          -Wno-unused-value
          >
     $<$<CXX_COMPILER_ID:MSVC>:
          /wd4996 # This function or variable may be unsafe.
          >
)