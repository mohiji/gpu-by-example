cmake_minimum_required(VERSION 3.15)

project(
    GBECommon
    VERSION 0.0.1
    LANGUAGES C)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
  PRIVATE
  Source/GBE_3DMath.c
  Source/GBE_Init.c
  Source/GBE_Shaders.c
)

# sets the search paths for the include files after installation
# as well as during when building the library (as these may differ)
# this allows the library itself and users to #include the library headers
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>)

