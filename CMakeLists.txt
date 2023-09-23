cmake_minimum_required(VERSION 3.16)

project(slow-balls LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/sdl)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/slowballs.cpp
)
target_link_options(${PROJECT_NAME} PRIVATE /STACK:6000000)
target_link_libraries(${PROJECT_NAME} SDL3-static)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
)
