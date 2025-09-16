#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/time.h>

#define CONNECTION_SIZE 1048576
#define MAX_POINTER 20
#define BUFFER_LENGTH 1024

#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

int epfd = 0;

typedef int (*RCALLBACK)(int fd);

int accept_cb(int fd);
int recv_cb(int fd);
int send_cb(int fd);

struct timeval begin;
struct timeval current;

struct conn {
    int fd;

    char rbuffer[BUFFER_LENGTH];
    int rlength;

    char wbuffer[BUFFER_LENGTH];
    int wlength;

    RCALLBACK send_callback;

    union {
        RCALLBACK recv_callback;
        RCALLBACK accept_callback;
    } r_action;
};

struct conn conn_list[CONNECTION_SIZE] = {0};

int set_event(int fd, int event, int flag) {
    if (flag) {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    } else {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
    }
 }


int event_register(int fd, int event)
{
    if (fd < 0) return -1;
    conn_list[fd].fd = fd;
    conn_list[fd].r_action.recv_callback = recv_cb;
    conn_list[fd].send_callback = send_cb;

    memset(conn_list[fd].rbuffer, 0, BUFFER_LENGTH);
    conn_list[fd].rlength = 0;
    memset(conn_list[fd].wbuffer, 0, BUFFER_LENGTH);
    conn_list[fd].wlength = 0;

    set_event(fd, event, 1);
}

int accept_cb(int fd) {

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);

    int clientfd = accept(fd, (struct sockaddr*)&clientaddr, &len);
    printf("accept client: %d\n", clientfd);
    if (clientfd < 0) {
        printf("accept errno: %d\n", errno);
        return -1;
    }

    event_register(clientfd, EPOLLIN);

    if((clientfd % 1000) == 0) {
        gettimeofday(&current, NULL);

		int time_used = TIME_SUB_MS(current, begin);
		memcpy(&begin, &current, sizeof(struct timeval));

		printf("accept finshed: %d, time_used: %d\n", clientfd, time_used);
    }

    return 0;
}

int recv_cb(int fd) {

    memset(conn_list[fd].rbuffer, 0, BUFFER_LENGTH );
    int count = recv(fd, conn_list[fd].rbuffer, BUFFER_LENGTH, 0);
    if (count == 0) {
        printf("client disconnected: %d\n", fd);
        close(fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    } else if (count < 0) {
        printf("recv errno: %d, %s\n", errno, strerror(errno));
        close(fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);

        return 0;
    }
    conn_list[fd].rlength = count;

    printf("RECV: %s\n", conn_list[fd].rbuffer);

#if 1
    conn_list[fd].wlength = conn_list[fd].rlength;
    memcpy(conn_list[fd].wbuffer, conn_list[fd].rbuffer, conn_list[fd].wlength);

#elif 0
    http_request(&conn_list[fd]);

#elif 0
    ws_request(&conn_list[fd]);

#endif
    set_event(fd, EPOLLOUT, 0);

    return count;
}

int send_cb(int fd) {

#if 0

    http_response(&conn_list[fd]);

#elif 0

    ws_response(&conn_list[fd]);

#endif

    int count = 0;
	if (conn_list[fd].wlength != 0) {
		count = send(fd, conn_list[fd].wbuffer, conn_list[fd].wlength, 0);
	}
	
	set_event(fd, EPOLLIN, 0);

    return count;
}

int Init_server(unsigned short port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
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

int main() {

    unsigned short port = 2000;
    
    epfd = epoll_create(1);

    int i = 0;

#if 1
    for (i = 0; i < MAX_POINTER; i++) {

        int sockfd = Init_server(port + i);
        conn_list[sockfd].fd = sockfd;
        conn_list[sockfd].r_action.accept_callback = accept_cb;
        set_event(sockfd, EPOLLIN, 1);
    }

#elif 1

    int sockfd = Init_server(port);
    conn_list[sockfd].fd = sockfd;
    conn_list[sockfd].r_action.recv_callback = accept_cb;
    set_event(sockfd, EPOLLIN, 1);

#endif

    gettimeofday(&begin, NULL);

    while(1) {

        struct epoll_event events[1024] = {0};
        int nready = epoll_wait(epfd, events, 1024, -1);

        int i = 0;
        for (i=0; i<nready; i++) {

            int connfd = events[i].data.fd;

            if (events[i].events & EPOLLIN) {
                conn_list[connfd].r_action.recv_callback(connfd);
            }

            if (events[i].events & EPOLLOUT) {
                conn_list[connfd].send_callback(connfd);
            }
        }


    }

}