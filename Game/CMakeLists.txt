cmake_minimum_required(VERSION 3.15)
project(Game)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp src/*.h src/*.hpp src/*.rc)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR}/src FILES ${SOURCE_FILES})

find_package(imgui REQUIRED)
find_package(Genesis REQUIRED)
find_package(soloud REQUIRED)

include_directories(
  src/ 
  ${GENESIS_INCLUDE_DIRS}
  ${IMGUI_INCLUDE_DIRS}
  ${SOLOUD_INCLUDE_DIRS}
)

link_directories(
  ${GENESIS_LIBRARY_DIRS}
  ${IMGUI_LIBRARY_DIRS}
  ${SOLOUD_LIBRARY_DIRS}
)

link_libraries(
  Genesis
  imgui
  soloud
)

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

if(WIN32)
    add_executable(Game WIN32 ${SOURCE_FILES})
    target_compile_definitions(Game PRIVATE UNICODE _UNICODE _HASEXCEPTIONS=0)
    target_compile_options(Game PUBLIC $<$<CONFIG:Debug>:/MTd> $<$<CONFIG:Release>:/MT>)
    target_compile_options(Game PUBLIC /MP) # Enable parallel compilation.
    target_compile_options(Game PUBLIC /Zi) # Debug information format: Program database.
    target_link_options(Game PRIVATE /DEBUG) # Enable PDB generation.
    target_link_options(Game PRIVATE $<$<CONFIG:Release>:/INCREMENTAL:NO /LTCG>)
    target_link_libraries(Game PRIVATE Opengl32 glu32 ws2_32)
    set_target_properties(Game PROPERTIES VS_STARTUP_PROJECT ${OUTPUT_DIRECTORY})
    set_target_properties(Game PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${OUTPUT_DIRECTORY})
else()
    add_executable(Game ${SOURCE_FILES})
    target_link_libraries(Game PRIVATE GL pthread asound)
endif()

target_link_libraries(Game PRIVATE GLEW::GLEW)
target_link_libraries(Game PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2-static)
target_link_libraries(Game PRIVATE SDL2::SDL2_image)
target_link_libraries(Game PRIVATE LinearMath Bullet3Common BulletDynamics BulletCollision)

target_compile_definitions(Game PRIVATE $<$<CONFIG:Debug>:_DEBUG>)

set_target_properties(Game PROPERTIES OUTPUT_NAME "Hexterminate")
set_target_properties(Game PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
set_target_properties(Game PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})

option(USE_STEAM "Use the Steam integration" OFF)
if(USE_STEAM)
    message(STATUS "Using Steam integration.")
    find_package(steamworks REQUIRED)
    target_include_directories(Game PRIVATE ${STEAMWORKS_INCLUDE_DIRS})
    target_compile_definitions(Game PRIVATE USE_STEAM=1 STEAM_APP_ID=1123230)
    target_link_directories(Game PRIVATE ${STEAMWORKS_LIBRARY_DIRS})

    if(WIN32)
        target_link_libraries(Game PRIVATE steam_api64)
        add_custom_command(
            TARGET Game 
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${STEAMWORKS_LIBRARY_DIRS}/steam_api64.dll ${OUTPUT_DIRECTORY}
        )
    else()
        target_link_libraries(Game PRIVATE steam_api)
    endif()
endif()

# Crash reporter support
add_custom_command(
    TARGET Game 
    POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIRECTORY}/crashhandler/
    COMMAND ${CMAKE_COMMAND} -E copy ${GENESIS_CRASH_HANDLER} ${OUTPUT_DIRECTORY}/crashhandler/
)

if (HEXTERMINATE_BUILD_VERSION)
    message(STATUS "Game version set: ${HEXTERMINATE_BUILD_VERSION}")
    target_compile_definitions(Game PRIVATE HEXTERMINATE_BUILD_VERSION=${HEXTERMINATE_BUILD_VERSION})
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
     target_compile_options(Game PRIVATE
          -Wall 
          -Werror
          -Wno-reorder-ctor
          -fstandalone-debug
     )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
     target_compile_options(Game PRIVATE
          -Wall 
          -Werror
          -Wno-reorder-ctor
     )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
     target_compile_options(Game PRIVATE
          /W4
          /WX # warnings as errors
          /wd4100 # unreferenced formal parameter
          /wd4121 # alignment of a member was sensitive to packing
          /wd4127 # conditional expression is constant
          /wd4505 # unreferenced local function has been removed
          /wd4189 # local variable is initialized but not referenced
          /wd4201 # nonstandard extension used : nameless struct/union
          /wd4702 # unreachable code
     )
endif()