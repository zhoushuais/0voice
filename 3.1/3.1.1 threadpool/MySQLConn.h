#pragma once

#include <cppconn/driver.h>
#include <string>

template <typename T>
class BlockingQueue;

namespace sql {
    class Driver;
    class Connection;
    class SQLException;
    class ResultSet;
}

class MySQLWorker;

struct MySQLConnInfo {

    explicit MySQLConnInfo(const std::string &info, const std::string &db) : url(url), database(db) {}

    std::string user;
    std::string password;
    std::string database;
    std::string url;
};


class MySQLConn {

public:
    MySQLConn(const std::string &info, const std::string &db, BlockingQueue<SQLOperation *> &task_queue);
    ~MySQLConn();

    int Open();
    void Close();

    sql::ResultSet* Query(const std::string &sql);
    // void SetWorker(MySQLWorker *worker) { worker_ = worker; }

private:
    void HandlerException(sql::SQLException &e);
    sql::Driver *driver_;
    sql::Connection *conn_;
    MySQLWorker *worker_;
    MySQLConnInfo info_;

};