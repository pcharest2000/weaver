cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(NNGUI_EXTRA_LIBS "")
project(weaver)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopt-info-vec-optimized -m64 -O3 -march=native")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3 ")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -O3  -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps -O3  -march=native")
# Required core libraries on various platforms
if (WIN32)
  list(APPEND NNGUI_EXTRA_LIBS opengl32)
elseif (APPLE)
  find_library(cocoa_library Cocoa)
  find_library(opengl_library OpenGL)
  find_library(corevideo_library CoreVideo)
  find_library(iokit_library IOKit)
  list(APPEND NNGUI_EXTRA_LIBS ${cocoa_library} ${opengl_library} ${corevideo_library} ${iokit_library})
  list(APPEND NNGUI_EXTRA_SOURCE darwin.mm)
elseif(CMAKE_SYSTEM MATCHES "Linux")
  list(APPEND NNGUI_EXTRA_LIBS GL Xxf86vm Xrandr Xinerama Xcursor Xi X11 pthread dl rt)
endif()
# Required libraries for linking against nngui (all targets)
message(${NNGUI_EXTRA_LIBS})
aux_source_directory(src SRC_LIST)
aux_source_directory(src/sdlgui  SRC_LIST_UI)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${SRC_LIST_UI}  )

include_directories(inc inc/sdlgui)
#add_definitions(-DIMGDIR="../img/")

# This may set gnu++2a
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
find_package(Boost COMPONENTS thread REQUIRED)

# SDL2

include(FindPkgConfig)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
pkg_search_module(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)
PKG_SEARCH_MODULE(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
#find_package(OpenGL REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS} inc inc/sdlgui )
link_directories(${Boost_LIBRARY_DIR})
target_link_libraries(${PROJECT_NAME} ${NNGUI_EXTRA_LIBS} ${SDL2_LIBRARIES}  ${SDL2IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES} Boost::thread )

#add_library(sse4_1 INTERFACE IMPORTED)

