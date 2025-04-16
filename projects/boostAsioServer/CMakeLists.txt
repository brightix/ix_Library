cmake_minimum_required(VERSION 3.30)
project(boostTest)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCE
        "src/Logger/*.cpp"
        "src/ThreadPool_singleton/ThreadPool_singleton.cpp"
)

find_package(Boost REQUIRED COMPONENTS system filesystem)



include_directories(/home/ix/mysql-connector-c++-9.3.0-linux-glibc2.28-x86-64bit/include)
link_directories(/home/ix/mysql-connector-c++-9.3.0-linux-glibc2.28-x86-64bit/lib64)


add_executable(Client
        Client.cpp

        src/tcp_service/tcp_service.cpp
        src/Session/Session.cpp
        src/client/client.cpp
        src/Dispatcher/ClientDispatcher.cpp
        src/utility/JsonInterpreter/JsonInterpreter.cpp)

add_executable(Server ${SOURCE}
        Server.cpp
        src/tcp_service/tcp_service.cpp
        src/Session/Session.cpp
        src/server/server.cpp
        src/Dispatcher/ServerDispatcher.cpp
        src/utility/JsonInterpreter/JsonInterpreter.cpp
        src/utility/SqlConnectionPool/SqlConnectionPool.cpp
        src/utility/SqlConnectionPool/SqlConnectionPool.h
)
target_link_libraries(Server PRIVATE Boost::filesystem Boost::system mysqlcppconn)
