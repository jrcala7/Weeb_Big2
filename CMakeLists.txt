# CMakeList.txt : CMake project for Weeb_Big2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Weeb_Big2")

add_subdirectory (external/big2-stack)
add_subdirectory (external/assimp)
add_subdirectory (external/stb)

# Compile bgfx shaders into embedded headers.
add_shaders_directory(shaders WEEB_SHADERS_TARGET)

# Automatically collect all source files from the src/ directory.
file(GLOB_RECURSE WEEB_BIG2_SOURCES CONFIGURE_DEPENDS
  "src/*.cpp"
  "src/*.h"
)

# Add source to this project's executable.
add_executable (Weeb_Big2 ${WEEB_BIG2_SOURCES})
target_include_directories(Weeb_Big2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(Weeb_Big2 PRIVATE BIG2 assimp stb ${WEEB_SHADERS_TARGET})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Weeb_Big2 PROPERTY CXX_STANDARD 20)
endif()

# Copy the resources folder to the output directory after each build.
add_custom_command(TARGET Weeb_Big2 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/resources"
    "$<TARGET_FILE_DIR:Weeb_Big2>/resources"
  COMMENT "Copying resources to output directory"
)

# TODO: Add tests and install targets if needed.
