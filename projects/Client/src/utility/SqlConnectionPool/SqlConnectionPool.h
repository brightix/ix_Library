//
// Created by ix on 25-4-16.
//
#pragma once
#include <queue>
#include <mysql/jdbc.h>
namespace ix {
namespace utility {

class SqlConnectionPool {
    sql::mysql::MySQL_Driver* driver_;
    std::queue<std::shared_ptr<sql::Connection>> pool;
    std::mutex connect_pool_mutex;
    size_t max_size;
public:
    //获取实例
    static SqlConnectionPool& Instance();

    //获取连接
    std::shared_ptr<sql::Connection> get_connection();
    //释放连接
    void release_connections(std::shared_ptr<sql::Connection>&& conn);

    SqlConnectionPool(SqlConnectionPool&) = delete;
    SqlConnectionPool& operator=(SqlConnectionPool&) = delete;
private:
    SqlConnectionPool();
    ~SqlConnectionPool() = default;

};

class SqlConnRALL
{
    std::shared_ptr<sql::Connection> conn_;
    SqlConnectionPool& pool_;
public:
    //SqlConnRALL(std::shared_ptr<sql::Connection>&& conn, SqlConnectionPool& pool);
    //
    explicit SqlConnRALL(SqlConnectionPool &pool);
    //输入请求获取结果
    std::unique_ptr<sql::ResultSet> query(const std::string &query);

    void execute(const std::string &query);

    ~SqlConnRALL();
};


} // utility
} // ix
