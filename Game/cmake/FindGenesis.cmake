set(GENESIS_INCLUDE_DIRS "${Genesis_SOURCE_DIR}/src/")
set(GENESIS_LIBRARY_DIRS "${Genesis_SOURCE_DIR}/libs/Genesis/${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}/${CMAKE_BUILD_TYPE}")

if(WIN32)
    set(GENESIS_CRASH_HANDLER "${Genesis_SOURCE_DIR}/../build/$<IF:$<CONFIG:Debug>,Debug,Release>/crashpad_handler.exe")
else()
    set(GENESIS_CRASH_HANDLER "${Genesis_SOURCE_DIR}/../build/crashpad_handler")
endif()