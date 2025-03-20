cmake_minimum_required(VERSION 3.16)
project(gbe-example1-getting-started)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in ../Libraries/SDL3
add_subdirectory(../Libraries/SDL3 ./build EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(gbe-example1-getting-started
  Source/Example1.c)

# Link to the actual SDL3 library.
target_link_libraries(gbe-example1-getting-started SDL3::SDL3)
