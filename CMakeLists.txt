cmake_minimum_required(VERSION 3.25)

project(geardemo C CXX)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# включаем логирование для FetchContent
set(FETCHCONTENT_QUIET OFF) 

# отключаем сборку примеров в raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

# подтягиваем зависимости из указанного git репозитория
# если зависимости плохо идут с использованием FetchContent, то:
# - закомментировать FetchContent_Declare
# - закомментировать FetchContent_MakeAvailable
# - сходить по адресу https://github.com/raysan5/raylib и скачать код zip-архивом
# - разархивировать папку raylib-master в корень (на уровне с Readme.md)
# - раскомментировать add_subdirectory(raylib-master)


FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG master)
FetchContent_MakeAvailable(raylib)
# add_subdirectory(raylib-master)

file(GLOB_RECURSE LIBS CONFIGURE_DEPENDS "src/libs/*.cpp")

set(SOURCE src/main.cpp)

set(PROJECT_INCLUDE
    src/include/
)

set(PROJECT_SOURCES
    src/libs/
    src/main.cpp
)


file(COPY src/resources 
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)

if(POLICY CMP0003)
    cmake_policy(SET CMP0003 NEW)
endif()

if(EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1 --preload-file resources --bind")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif ()


add_executable(${PROJECT_NAME}) 
target_sources(${PROJECT_NAME} PRIVATE ${SOURCE} ${LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
