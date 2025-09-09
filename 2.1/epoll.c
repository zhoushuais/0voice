#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>


int main() {

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0000
    servaddr.sin_port = htons(2000); // 1-1023系统默认使用
    if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)))
    {
        printf("bind failed: %s \n", strerror(errno));
    }

    listen(sockfd, 10); // 最大连接数
    printf("listen finshed: %d\n", sockfd);

    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
#if 0

#else
    int epfd = epoll_create(1);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    while(1) {
        
        struct epoll_event events[1024] = {0};
        int nready = epoll_wait(epfd, events, 1024, -1);

        int i = 0;
        for (i = 0; i < nready; i++) {

            int connfd = events[i].data.fd;

            if (connfd == sockfd) {

                int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
                printf("accept client: %d\n", clientfd);

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

            } else if (events[i].events & EPOLLIN) {

                char buffer[1024] = {0};
                int count = recv(connfd, buffer, 1024, 0);
                if (count == 0) {
                    printf("client disconnected: %d\n", connfd);
                    close(connfd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);

                    continue;
                }

                printf("RECV: %s\n", buffer);

                count = send(i, buffer, count, 0);
                printf("SEND: %d bytes\n", count);
            }
        }
    }

#endif   
    getchar();

    printf("exit\n");

    return 0;


}