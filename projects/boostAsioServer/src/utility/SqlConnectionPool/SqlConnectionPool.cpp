//
// Created by ix on 25-4-16.
//

#include "SqlConnectionPool.h"

namespace ix {
namespace utility {
using namespace std;
SqlConnectionPool& SqlConnectionPool::Instance()
{
    static SqlConnectionPool instance = SqlConnectionPool();
    return instance;
}

SqlConnectionPool::SqlConnectionPool() : max_size(10),driver_(sql::mysql::get_driver_instance())
{
    for (int i = 0;i < max_size; i++)
    {
        pool.push(shared_ptr<sql::Connection>(driver_->connect("tcp://127.0.0.1:3306","root","123")));
    }
}

std::shared_ptr<sql::Connection> SqlConnectionPool::get_connection()
{
    shared_ptr<sql::Connection> conn;
    {
        lock_guard<mutex> lock(connect_pool_mutex);
        conn = pool.front();
        pool.pop();
    }
    return conn;
}

void SqlConnectionPool::release_connections(std::shared_ptr<sql::Connection>&& conn)
{
    {
        lock_guard<mutex> lock(connect_pool_mutex);
        pool.emplace(move(conn));
    }
}



SqlConnRALL::SqlConnRALL(SqlConnectionPool &pool) : pool_(pool)
{
    conn_ = pool_.get_connection();
}

std::unique_ptr<sql::ResultSet> SqlConnRALL::query(const string& query)
{
    conn_->setSchema("my_database");
    unique_ptr<sql::Statement> stmt(conn_->createStatement());

    return std::unique_ptr<sql::ResultSet>(stmt->executeQuery(query));
}

SqlConnRALL::~SqlConnRALL()
{
    pool_.release_connections(std::move(conn_));
}
} // utility
} // ix