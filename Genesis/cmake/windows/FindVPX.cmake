set(VPX_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/Genesis/libs/libvpx-v1.10.0/include" CACHE STRING "VPX - include directory")
set(VPX_LIBRARY_DIRS "${CMAKE_SOURCE_DIR}/Genesis/libs/libvpx-v1.10.0/lib/x86" CACHE STRING "VPX - library directory")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VPX REQUIRED_VARS VPX_INCLUDE_DIRS VPX_LIBRARY_DIRS)