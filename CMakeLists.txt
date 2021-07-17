if(WIN32)
  set(VCPKG_TARGET_TRIPLET "x64-windows-static")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_LIST_DIR}/overlay_ports")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

cmake_minimum_required(VERSION 3.20)
project(tiny-drive-prof)

add_library(dependencies INTERFACE)

find_path(CR_INCLUDE_DIR NAMES cr.h)
target_include_directories(dependencies INTERFACE ${CR_INCLUDE_DIR})

find_package(imgui REQUIRED)
target_link_libraries(dependencies INTERFACE imgui::imgui)

find_package(implot REQUIRED)
target_link_libraries(dependencies INTERFACE implot::implot)

find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h)
find_library(GLFW_LIBRARY glfw3)
target_include_directories(dependencies INTERFACE ${GLFW_INCLUDE_DIR})
target_link_libraries(dependencies INTERFACE ${GLFW_LIBRARY})

find_library(GL3W_LIBRARY gl3w)
string(REPLACE "/debug/lib/" "$<$<CONFIG:Debug>:/debug>/lib/" GL3W_LIBRARY ${GL3W_LIBRARY})
message(STATUS GL3W_LIBRARY=${GL3W_LIBRARY})
target_link_libraries(dependencies INTERFACE ${GL3W_LIBRARY})

find_path(THREAD_POOL_INCLUDE_DIR thread_pool.hpp)
target_include_directories(dependencies INTERFACE ${THREAD_POOL_INCLUDE_DIR})

find_package(efsw REQUIRED)
target_link_libraries(dependencies INTERFACE efsw::efsw)

# TODO: Config-specific lib paths (now debug)

set(TDP_GUEST ${CMAKE_SHARED_LIBRARY_PREFIX}tdp-guest${CMAKE_SHARED_LIBRARY_SUFFIX})
add_library(guest SHARED
  src/guest/App.cpp
  src/guest/App.h
  src/guest/Controller.h
  src/guest/DirectoryIndexer.cpp
  src/guest/DirectoryIndexer.h
  src/guest/DirectoryInfo.h
  src/guest/main.cpp
  src/guest/Model.h
  src/guest/MyController.cpp
  src/guest/MyController.h
  src/guest/SearchTask.cpp
  src/guest/SearchTask.h
  src/guest/View.cpp
  src/guest/View.h
  src/HostData.h
  .clang-format
)
target_link_libraries(guest PUBLIC dependencies)
target_include_directories(guest PUBLIC ${CMAKE_SOURCE_DIR}/src)
add_custom_command(
  TARGET guest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:guest> $<TARGET_FILE_DIR:guest>/${TDP_GUEST}
)

add_executable(host
  src/host/main.cpp
  src/HostData.h
  .clang-format
)
target_link_libraries(host PUBLIC dependencies)
target_include_directories(host PUBLIC ${CMAKE_SOURCE_DIR}/src)
add_dependencies(host guest)
set_target_properties(host PROPERTIES OUTPUT_NAME tiny-drive-prof)
add_custom_command(
  TARGET host POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/fonts/arial.ttf $<TARGET_FILE_DIR:guest>/arial.ttf
)

target_compile_definitions(host PUBLIC TDP_GUEST=\"${TDP_GUEST}\")