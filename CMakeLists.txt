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

# Add source to this project's executable.
add_executable (Weeb_Big2
  "Weeb_Big2.cpp"
  "Weeb_Big2.h"
  "AppWindow.h"
  "AppWindow.cpp"
)

target_link_libraries(Weeb_Big2 PRIVATE BIG2 assimp)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Weeb_Big2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
