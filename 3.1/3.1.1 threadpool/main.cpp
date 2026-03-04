#include "MySQLConnPool.h"
#include "AsyncProcessor.h"
#include <cppconn/resultset.h>
#include <iostream>
#include <thread>
#include <chrono>

void HandleQueryResult(std::unique_ptr<sql::ResultSet> res) {
    while (res->next()) {
        std::cout << "cid: " << res->getInt("cid") << " caption: " << res->getString("caption") << std::endl;
    }
}

int main() {
    MySQLConnPool *pool1 = MySQLConnPool::GetInstance("edu_svc");
    pool1->InitPool("tcp://127.0.0.1:3306;root;123456", 10);
    AsyncProcessor response_handler;

    auto query_callback1 = pool1->Query("SELECT * FROM class", HandleQueryResult);
    response_handler.AddQueryCallback(std::move(query_callback1));

    while (true) {
        response_handler.InvokeIfReady();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}