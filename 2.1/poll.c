#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>
#include <poll.h>

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

    struct pollfd fds[1024] = {0};
    fds[sockfd].fd = sockfd;
    fds[sockfd].events = POLLIN;

    int maxfd = sockfd;

    while(1) {
        int nready = poll(fds, maxfd+1, -1);  // 第三个参数是超时时间，-1表示无限等待

        if (fds[sockfd].revents & POLLIN) {

            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            printf("accept client: %d\n", clientfd);

            fds[clientfd].fd = clientfd;
            fds[clientfd].events = POLLIN;

            if (clientfd > maxfd) maxfd = clientfd;
        }

        int i = 0;
        for (i = sockfd + 1; i <= maxfd; i++) {
            if (fds[i].revents & POLLIN) {

                char buffer[1024] = {0};
                int count = recv(i, buffer, 1024, 0);
                if (count == 0) {
                    printf("client disconnected: %d\n", i);
                    close(i);

                    fds[i].fd = -1;
                    fds[i].events = 0;
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