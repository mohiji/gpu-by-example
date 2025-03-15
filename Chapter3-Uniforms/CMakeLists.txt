cmake_minimum_required(VERSION 3.16)
project(gbe-chapter3-uniforms)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in ../Libraries/SDL3
add_subdirectory(../Libraries/SDL3 ./build EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(gbe-chapter3-uniforms
  Source/main.c
  Source/AppContext.c
  Source/GBE_3DMath.c
  Source/GBE_Shaders.c)

# Link to the actual SDL3 library.
target_link_libraries(gbe-chapter3-uniforms m SDL3::SDL3)

