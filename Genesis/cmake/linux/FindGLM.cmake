set(GLM_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/Genesis/libs/glm/include" CACHE STRING "GLM - include directory")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLM REQUIRED_VARS GLM_INCLUDE_DIRS)