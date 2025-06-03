cmake_minimum_required(VERSION 3.31)
project(tornado_player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)



include_directories(/usr/include)
link_directories(/lib/x86_64-linux-gnu)
find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        Concurrent
        REQUIRED)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

qt5_add_resources(RESOURCES resources.qrc)
add_executable(tornado_player
        main.cpp
        app/MainWindow.cpp
        src/ffmpeg/DecoderWorker.cpp
        src/utils/ThreadPool_singleton/ThreadPool_singleton.cpp
        app/MyApp.h
        app/MyApp.cpp
        src/RenderThread/RenderThread.cpp
        src/AudioThread/AudioThread.cpp
)
target_sources(tornado_player PRIVATE ${RESOURCES})

target_link_libraries(tornado_player
        PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::Concurrent
        avformat
        avcodec
        avutil
        swscale
        swresample
        ${SDL2_LIBRARIES}
)

