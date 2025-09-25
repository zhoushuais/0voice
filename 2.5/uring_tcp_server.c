#include <stdio.h>
#include <liburing.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define ENTRIES_LENGTH 1024
#define BUFFER_LENGTH 1024
#define EVENT_ACCEPT 0
#define EVENT_READ 1
#define EVENT_WRITE 2

struct conn_info {
    int fd;
    int event;
};

int Init_server(unsigned short port) {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0000
    servaddr.sin_port = htons(port); // 1-1023系统默认使用
    if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)))
    {
        printf("bind failed: %s \n", strerror(errno));
    }

    listen(sockfd, 10); // 最多有 10 个未被 accept 的连接在队列里
    printf("listen finshed: %d\n", sockfd);

    return sockfd;
}

int set_event_accept(struct io_uring *ring, int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) {

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);  // 从 ring 的 SQ 中获取一个空闲的 sqe（供用户填充请求）

    struct conn_info accept_info = {
        .fd = sockfd,
        .event = EVENT_ACCEPT,
    };

    io_uring_prep_accept(sqe, sockfd, (struct sockaddr*)addr, addrlen, flags);   // 填充 sqe 为一个异步 accept 请求
    memcpy(&sqe->user_data, &accept_info, sizeof(struct conn_info));
}

int set_event_recv(struct io_uring *ring, int sockfd, void *buf, size_t len, int flags) {

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);  // 从 ring 的 SQ 中获取一个空闲的 sqe（供用户填充请求）

    struct conn_info accept_info = {
        .fd = sockfd,
        .event = EVENT_READ,
    };

    io_uring_prep_recv(sqe, sockfd, buf, len, flags);   // 填充 sqe 为一个异步 accept 请求
    memcpy(&sqe->user_data, &accept_info, sizeof(struct conn_info));
}

int set_event_send(struct io_uring *ring, int sockfd, void *buf, size_t len, int flags) {

    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);  // 从 ring 的 SQ 中获取一个空闲的 sqe（供用户填充请求）

    struct conn_info accept_info = {
        .fd = sockfd,
        .event = EVENT_WRITE,
    };

    io_uring_prep_send(sqe, sockfd, buf, len, flags);   // 填充 sqe 为一个异步 accept 请求
    memcpy(&sqe->user_data, &accept_info, sizeof(struct conn_info));
}

int main(int argc, char *argv[]) {

    unsigned short port = 9999;
    int sockfd = Init_server(port);

    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    // io_uring 的初始化参数结构体，用于配置 io_uring 的特性（如是否启用 SQ 轮询、是否使用固定缓冲区等）。
    // 这里用 memset 清零，采用默认配置

    struct io_uring ring;  // io_uring 的核心结构体，内部包含提交队列（SQ）、完成队列（CQ）的元数据（如队列指针、大小、共享内存地址等）
    io_uring_queue_init_params(ENTRIES_LENGTH, &ring, &params);  // 初始化 io_uring 实例的函数,该函数会在内核中创建 SQ 和 CQ，并将它们的共享内存地址映射到用户态，供后续操作

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    set_event_accept(&ring, sockfd, (struct sockaddr*)&clientaddr, &len, 0);

    char buffer[BUFFER_LENGTH] = {0};

    while(1) {

        io_uring_submit(&ring);     // 将 SQ 中已填充的请求（这里是之前准备的 accept 请求）提交给内核处理

        struct io_uring_cqe *cqe;
        io_uring_wait_cqe(&ring, &cqe);

        struct io_uring_cqe *cqes[128];
        int nready = io_uring_peek_batch_cqe(&ring, cqes, 128);

        int i = 0;
        for (i = 0; i < nready; i++) {

            struct io_uring_cqe *entries = cqes[i];
            struct conn_info result;
            memcpy(&result, &entries->user_data, sizeof(struct conn_info));

            if (result.event == EVENT_ACCEPT) {
                // 处理“接受连接”完成事件
                // a. 立即添加新的 accept 请求（确保能继续接受其他客户端）
                set_event_accept(&ring, sockfd, (struct sockaddr*)&clientaddr, &len, 0);
                printf("set_event_accept\n");

                int connfd = entries->res;

                set_event_recv(&ring, connfd, buffer, BUFFER_LENGTH, 0);  // 为新连接添加读请求（等待客户端发送数据）
            } else if (result.event == EVENT_READ) {
                int ret = entries->res;
                printf("set_event_recv: %d, %s\n", ret, buffer);

                if (ret == 0) {
                    close(result.fd);

                } else if (ret > 0) {

                    set_event_send(&ring, result.fd, buffer, ret, 0);
                }

            } else if (result.event == EVENT_WRITE) {
                int ret = entries->res;
                printf("set_event_send ret: %d, %s \n", ret, buffer);

                set_event_recv(&ring, result.fd, buffer, BUFFER_LENGTH, 0);
            }
        }

        io_uring_cq_advance(&ring, nready);  // 通知内核：已经处理完 nready 个 CQE，内核可以释放这些 CQE 占用的空间，供后续新的完成结果使用

    }

}