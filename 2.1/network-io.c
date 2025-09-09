#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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

    while(1) {
        int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
        printf("accept client: %d\n", clientfd);

        pthread_t tid;
        pthread_create(&tid, NULL, cllent_thread, (void *)&clientfd);
    }
    
    getchar();

    printf("exit\n");

    return 0;
}