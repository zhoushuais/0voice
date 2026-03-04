

#include "MySQLConnPool.h"
#include "MySQLConn.h"
#include "SQLOperation.h"
#include "QueryCallback.h"
#include <cppconn/resultset.h>
#include "blockingqueue.h"

std::unordered_map<std::string, MySQLConnPool *> MySQLConnPool::instances_;

MySQLConnPool *MySQLConnPool::GetInstance(const std::string &db) {
    if (instances_.find(db) == instances_.end()) {
        instances_[db] = new MySQLConnPool(db);
    }
    return instances_[db];
}

void MySQLConnPool::InitPool(const std::string &url, int pool_size) {
    task_queue_ = new BlockingQueue<SQLOperation *>();
    for (int i = 0; i < pool_size; ++i) {
        MySQLConn *conn = new MySQLConn(url, database_, *task_queue_);
        conn->Open();
        pool_.push_back(conn);
    }
}

MySQLConnPool::~MySQLConnPool() {
    if (task_queue_)
        task_queue_->Cancel();
    for (auto conn : pool_) {
        delete conn;
    }
    if (task_queue_) {
        delete task_queue_;
        task_queue_ = nullptr;
    }
    pool_.clear();
}


QueryCallback MySQLConnPool::Query(const std::string &sql, std::function<void(std::unique_ptr<sql::ResultSet>)> &&cb) {
    SQLOperation *op = new SQLOperation(sql);
    auto future = op->GetFuture();
    task_queue_->Push(op);
    return QueryCallback(std::move(future), std::move(cb));
}
