#pragma once

#include <thread>


class MySQLConn;

template <typename T>
class BlockingQueue;

class SQLOperation;


class MySQLWorker {
public:
    MySQLWorker(MySQLConn *conn, BlockingQueue<SQLOperation *> &task_queue);
    ~MySQLWorker();

    void start();
    void stop();


private:

    void Worker();
    MySQLConn *conn_;
    std::thread worker_;
    BlockingQueue<SQLOperation *> &task_queue_;

};