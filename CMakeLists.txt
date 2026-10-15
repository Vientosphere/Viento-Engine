cmake_minimum_required(VERSION 3.20)
project(VientoEngine VERSION 0.1.0 LANGUAGES CXX)

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_library(viento_core
    CameraController.cpp
    Scene.cpp
)
target_include_directories(viento_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(viento_core PUBLIC raylib)

add_executable(viento_engine
    main.cpp
    Engine.cpp
)
target_link_libraries(viento_engine PRIVATE viento_core raylib)

if(BUILD_TESTING)
    add_executable(viento_core_tests tests/core_tests.cpp)
    target_link_libraries(viento_core_tests PRIVATE viento_core raylib)
    add_test(NAME viento_core_tests COMMAND viento_core_tests)
endif()
