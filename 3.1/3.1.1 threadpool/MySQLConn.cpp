#include "MySQLConn.h"
#include "QueryCallback.h"
#include "MySQLWorker.h"
#include "blockingqueue.h"

#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

#include <vector>
#include <string>
#include <string_view>

class SQLOperation;
class MySQLWorker;

// "tcp://127.0.0.1:3306;root;123456"
static std::vector<std::string_view>
Tokenize(std::string_view str, char sep, bool keepEmpty) {
    std::vector<std::string_view> tokens;

    size_t start = 0;
    for (size_t end = str.find(sep); end != std::string_view::npos; end = str.find(sep, start))
    {
        if ((end > start) || keepEmpty) {
            tokens.emplace_back(str.substr(start, end - start));
        }
        start = end + 1;
    }

    if (keepEmpty || (start < str.length()))
        tokens.emplace_back(str.substr(start));

    return tokens;
}



MySQLConnInfo::MySQLConnInfo(const std::string &info, const std::string &db) {
    auto tokens = Tokenize(info, ';', false);
    if (tokens.size() != 3)
        return;

    url.assign(tokens[0]);
    user.assign(tokens[1]);
    password.assign(tokens[2]);
    database.assign(db);

}


MySQLConn::MySQLConn(const std::string &info, const std::string &db, BlockingQueue<SQLOperation *> &task_queue) : info_(info, db) {

    worker_ = new MySQLWorker(this, task_queue);
    worker_->Start();
}

MySQLConn::~MySQLConn() {
    if (worker_) {
        worker_->Stop();
        delete worker_;
        worker_ = nullptr;
    }

    if (conn_) {
        delete conn_;
    }

}

int MySQLConn::Open() {
    int err = 0;
    try {
        driver = get_driver_instance();
        conn_ = driver->connect(info_.url, info_.user, info_.password);
        if (!conn_) {
            return -1;
        }

        conn_->setSchema(info_.database);
    } catch (sql::SQLException &e) {
        HandlerException(e);
        err = e.getErrorCode(); 
    }

    return err;
}

void MySQLConn::Close() {
    if (conn_) {
        conn_->close();
        delete conn_;
        conn_ = nullptr;
    }
}

sql::ResultSet* MySQLConn::Query(const std::string &sql) {
    try {
        sql::Statement *stmt = conn_->createStatement();
        return stmt->executeQuery(sql);
    } catch (sql::SQLException &e) {
        HandlerException(e);
    }

    return nullptr;
}


void MySQLConn::HandlerException(sql::SQLException &e) {
    if (e.getErrorCode() != 0) {

    }
}