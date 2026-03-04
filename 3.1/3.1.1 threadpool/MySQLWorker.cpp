#include "MySQLWorker.h"

#include "blockingqueue.h"
#include "MySQLConn.h"
#include "SQLOperation.h"


MySQLWorker::MySQLWorker(MySQLConn *conn, BlockingQueue<SQLOperation *> &task_queue) : conn_(conn), task_queue_(task_queue) {

}

MySQLWorker::~MySQLWorker() {
    stop();
}

void MySQLWorker::start() {
    worker_ = std::thread(&MySQLWorker::Worker, this);
}

void MySQLWorker::stop() {
    if (worker_.joinable())
        worker_.join();
}


void MySQLWorker::Worker() {
    while (true) {
        SQLOperation *op = nullptr;
        if (!task_queue_.Pop(op)) {
            break;
        }
        op->Execute(conn_);
        delete op;
    }
}