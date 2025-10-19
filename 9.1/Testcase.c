
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MSG_LENGTH  1024

int send_msg(int connfd, char *msg, int length) {

    int res = send(connfd, msg, length, 0);
    if (res < 0) {
        perror("send error");
        exit(1);
    }
    return res;
}


int recv_msg(int connfd, char *msg, int length) {
    int res = recv(connfd, msg, length, 0);
    if (res < 0) {
        perror("recv error");
        exit(1);
    }
    return res;
}

void testcase(int connfd, char *msg, char *pattern, char *casename) {

    if (!msg || !pattern || !casename) return;

    send_msg(connfd, msg, strlen(msg));

    char result[MAX_MSG_LENGTH] = {0};
    recv_msg(connfd, result, MAX_MSG_LENGTH);

    if (strcmp(result, pattern) == 0) {
        printf("case %s passed\n", casename);
    } else {
        printf("case %s failed, expect: %s, get: %s\n", casename, pattern, result);
    }
}


int connect_tcpserver(const char *ip, unsigned short port) {

    int connfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(ip);
    serveraddr.sin_port = htons(port);


    if ( 0 != connect(connfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in))) {
        perror("connect error");
        return -1;
    }

    return connfd;
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("argc error\n");
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int connfd = connect_tcpserver(ip, port);

    testcase(connfd, "SET Teacher King", "OK\r\n", "SET-Teacher");
    testcase(connfd, "GET Teacher", "King\r\n", "GET-Teacher");

    return 0;
}