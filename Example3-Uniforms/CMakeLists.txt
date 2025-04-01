cmake_minimum_required(VERSION 3.16)
project(gbe-example3-uniforms)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in ../Libraries/SDL3
add_subdirectory(../Libraries/SDL3 ./build-SDL3 EXCLUDE_FROM_ALL)

# Common code has been factored out to a little shared library
add_subdirectory(../GBECommon ./build-GBECommon EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(${PROJECT_NAME} Source/Example3.c)

# Link to the actual SDL3 library.
target_link_libraries(${PROJECT_NAME} SDL3::SDL3 GBECommon m)

