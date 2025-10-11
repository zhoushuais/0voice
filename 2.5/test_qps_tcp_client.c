#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

typedef struct test_context_s {
    char serverip[16];
    int port;
    int threadnum;   // 线程数量
    int connection;  // 连接的数量
    int requestion;  // 发送的请求数量

    int failed;
} test_context_t;

int connect_tcpserver(const char *ip, unsigned short port) {

    int connfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in tcpserver_addr;
    memset(&tcpserver_addr, 0, sizeof(struct sockaddr_in));

    tcpserver_addr.sin_family = AF_INET;
    tcpserver_addr.sin_addr.s_addr = inet_addr(ip);  
    tcpserver_addr.sin_port = htons(port);

    int ret = connect(connfd, (struct sockaddr*)&tcpserver_addr, sizeof(struct sockaddr_in));
    if (ret) {
        perror("connect\n");
        return -1;
    }

    return connfd;
}

#define TEST_MESSAGE "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyz\r\n"
#define RBUFFER_LENGTH 2048
#define WBUFFER_LENGTH 2048

int send_recv_tcppkt(int fd) {

#if 0
    int res = send(fd, TEST_MESSAGE, strlen(TEST_MESSAGE), 0);
    if (res < 0) {
        exit(1);
    }

    char rbuffer[RBUFFER_LENGTH] = {0};
    res = recv(fd, rbuffer, RBUFFER_LENGTH, 0);
    if (res < 0) {
        exit(1);
    }

    if(strcmp(rbuffer, TEST_MESSAGE) !=0) {
        printf("failed: '%s' != '%s'\n", rbuffer, TEST_MESSAGE);
    }

#elif

    char wbuffer[WBUFFER_LENGTH] = {0};
    int i = 0;

    for (i = 0; i < 16; i++) {
        strcpy(wbuffer + i * strlen(TEST_MESSAGE), TEST_MESSAGE);
    }

    int res = send(fd, wbuffer, strlen(wbuffer), 0);
    if (res < 0) {
        exit(1);
    }

    char rbuffer[RBUFFER_LENGTH] = {0};
    res = recv(fd, rbuffer, RBUFFER_LENGTH, 0);
    if (res < 0) {
        exit(1);
    }

    if(strcmp(rbuffer, wbuffer) !=0) {
        printf("failed: '%s' != '%s'\n", rbuffer, wbuffer);
        return -1;
    }

#endif
    return 0;

}

static void *test_qps_entry(void *arg) {
    test_context_t *pctx = (test_context_t *)arg;
    int connfd = connect_tcpserver(pctx->serverip, pctx->port);
    if (connfd < 0) {
        printf("connect tcp server failed\n");
        return NULL;
    }

    int count = pctx->requestion / pctx->threadnum;
    int i = 0;

    int res;

    while (i++ < count) {
        res = send_recv_tcppkt(connfd);
        if (res != 0) {
            printf("send_recv_tcppkt failed\n");
            pctx->failed++;
            continue;
        }
    }

    return NULL;

}

int main(int argc, char *argv[]) {

    int ret = 0;
    test_context_t ctx = {0};

    int opt;
    while ((opt = getopt(argc, argv, "s:p:t:c:n:?")) != -1) {
        switch(opt) {
            case 's':
                printf("-s: %s\n", optarg);
                strcpy(ctx.serverip, optarg);
                break;
            case 'p':
                printf("-p: %s\n", optarg);
                ctx.port = atoi(optarg);
                break;
            case 't':
                printf("-t: %s\n", optarg);
                ctx.threadnum = atoi(optarg);
                break;
            case 'c':
                printf("-c: %s\n", optarg);
                ctx.connection = atoi(optarg);
                break;
            case 'n':
                printf("-n: %s\n", optarg);
                ctx.requestion = atoi(optarg);
                break;
            default:
                return -1;
        }
    }

    pthread_t *ptid = malloc(sizeof(pthread_t) * ctx.threadnum);

    int i = 0;

    struct timeval tv_begin;
    gettimeofday(&tv_begin, NULL);

    for (i = 0; i < ctx.threadnum; i++) {
        pthread_create(&ptid[i], NULL, test_qps_entry, &ctx);
    }

    for (i = 0; i < ctx.threadnum; i++) {
        pthread_join(ptid[i], NULL);
    }

    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);

    int time_used = TIME_SUB_MS(tv_end, tv_begin);

    printf("success: %d, failed: %d, time used: %d ms, qps: %d\n", ctx.requestion - ctx.failed, ctx.failed, time_used, ctx.requestion * 1000 / time_used);


clean:
    free(ptid);

    return ret;

}