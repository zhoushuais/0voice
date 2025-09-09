#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/select.h>

void *cllent_thread(void *arg) {

    int clientfd = *(int *)arg;

    while(1) {
        char buffer[1024] = {0};
        int count = recv(clientfd, buffer, 1024, 0);
        if (count == 0) {
            printf("client disconnected: %d\n", clientfd);
            close(clientfd);
            break;
        }

        printf("RECV: %s\n", buffer);

        count = send(clientfd, buffer, count, 0);
        printf("SEND: %d bytes\n", count);
    }
}

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
    fd_set rfds, rset;

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    int maxfd = sockfd;  // 最大文件描述符,用来遍历所有文件描述符

    while(1) {
        rset = rfds;

        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);   // 5个参数，第一个是最大的文件描述符，第二个是读文件描述符集合，
                    // 第三个是写文件描述符集合，第四个是异常文件描述符集合，第五个是等待时间

        if (FD_ISSET(sockfd, &rset)) {

            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
            printf("accept client: %d\n", clientfd);

            FD_SET(clientfd, &rfds);

            if (clientfd > maxfd) maxfd = clientfd;
        }

        int i = 0;
        for (i = sockfd + 1; i <= maxfd; i++) {
            if (FD_ISSET(i, &rset)) {

                char buffer[1024] = {0};
                int count = recv(i, buffer, 1024, 0);
                if (count == 0) {
                    printf("client disconnected: %d\n", i);
                    close(i);
                    FD_CLR(i, &rfds);
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