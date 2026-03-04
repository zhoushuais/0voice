#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

#include "QueryCallback.h"

#include <cppconn/resultset.h>


namespace sql {
    class ResultSet;
}

class MySQLConn;

template <typename T>
class BlockingQueue;

class SQLOperation;


class MySQLConnPool {

public:
    static MySQLConnPool *GetInstance (const std::string &db);

    void InitPool(const std::string &url, int pool_size);
    QueryCallback Query(const std::string &sql, std::function<void(std::unique_ptr<sql::ResultSet>)> &&cb);



private:
    MySQLConnPool(const std::string &db) : database_(db) {}
    ~MySQLConnPool();

    std::string database_;
    std::vector<MySQLConn *> pool_;
    static std::unordered_map<std::string, MySQLConnPool *> instances_;
    BlockingQueue<SQLOperation *> conn_queue_;
};