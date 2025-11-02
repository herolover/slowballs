cmake_minimum_required(VERSION 3.28)

project(slow-balls LANGUAGES CXX)

find_package(OpenMP)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/OPT:REF /OPT:ICF")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/sdl)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp
)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} SDL3-static OpenMP::OpenMP_CXX)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
)
