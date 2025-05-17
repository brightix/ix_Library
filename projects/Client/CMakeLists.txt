cmake_minimum_required(VERSION 3.31)
project(boostAsioClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

add_executable(boostAsioClient
        main.cpp
        src/tcp_service/tcp_service.cpp
        src/Session/Session.cpp
        src/client/client.cpp
        src/Dispatcher/ClientDispatcher.cpp
        src/utility/JsonInterpreter/JsonInterpreter.cpp
        app/MainWindow.cpp
        app/LoginWidget.cpp
        app/LoginWidget.h
        app/LoginWidget.ui
        app/SignInWidget.cpp
        app/SignInWidget.h
        app/SignInWidget.ui
)


target_link_libraries(boostAsioClient
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)

