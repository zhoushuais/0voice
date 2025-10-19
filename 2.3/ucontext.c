#define _GNU_SOURCE
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/epoll.h>


ucontext_t ctx[3];
ucontext_t main_ctx;

int count = 0;

// hook
// typedef 返回值类型 (*新类型名)(参数列表) 是定义函数指针类型的固定格式
// ssize_t：有符号整数类型
typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
read_t read_f;

typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
write_t write_f;


//代码重定义了read和write函数，它们的参数列表和返回值与系统标准read/write完全一致，这是实现 "无缝替换" 的关键
ssize_t read(int fd, void *buf, size_t count) {
    
    struct pollfd fds[1] = {0};

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    int res = (poll(fds, 1, 0));
    if(res <= 0) {

        // fd -->epoll_ctl();
        swapcontext();
    }

    ssize_t ret = read_f(fd, buf, count);
    printf("read: %s\n", (char *)buf);
    return ret;


    ssize_t ret = read_f(fd, buf, count);
    printf("read: %s\n", (char *)buf);
    return ret;
}

ssize_t write(int fd, const void *buf, size_t count) {

    printf("write: %s\n", (const char *)buf);
    return write_f(fd, buf, count);
}

void Init_hook(void) {
    if (!read_f) {
        read_f = dlsym(RTLD_NEXT, "read");
        // read_f = (read_t)dlsym(RTLD_NEXT, "read"); // 从动态链接库中获取原始read函数地址
    }

    if (!write_f) {
        write_f = dlsym(RTLD_NEXT, "write");
    }

}

void func1(void) {

    while (count ++ < 30){
        printf("1\n");
        swapcontext(&ctx[0], &ctx[1]);
        printf("4\n");
    }
}

void func2(void) {

    while (count ++ < 30){
        printf("2\n");
        swapcontext(&ctx[1], &ctx[2]);
        printf("5\n");
    }
}

void func3(void) {

    while (count ++ < 30){
        printf("3\n");
        swapcontext(&ctx[2], &ctx[0]);
        printf("6\n");
    }
}

int main() {

    Init_hook();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct serveraddr_in));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(2048);
    serveraddr.sin_addr.s_addr = inet_addr(INADDR_ANY);

    if (-1 == bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(struct serveraddr_in))) {
        perror("bind error");
        return -1;
    }

    listen(sockfd, 5);

    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
    printf("client connected\n");

    while (1) {
        char buffer[128] = {0};
        int count = read(clientfd, buffer, 128);
        if (count <= 0) {
            break;
        }
    }

    // int fd = open("a.txt", O_CREAT | O_RDWR);
    // if (fd < 0) {
    //     return -1;
    // }

    // char *str = "1234567890";
    // write(fd, str, strlen(str));

    // char buffer[128] = {0};
    // read(fd, buffer, 128);
    // printf("buffer: %s\n", buffer);

#if 0

    char stack1[2048] = {0};

    char stack2[2048] = {0};

    char stack3[2048] = {0};

    getcontext(&ctx[0]);
    ctx[0].uc_stack.ss_sp = stack1;
    ctx[0].uc_stack.ss_size = sizeof(stack1);
    ctx[0].uc_link = &main_ctx;
    makecontext(&ctx[0], func1, 0); 

    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = stack2;
    ctx[1].uc_stack.ss_size = sizeof(stack2);
    ctx[1].uc_link = &main_ctx;
    makecontext(&ctx[1], func2, 0); 

    getcontext(&ctx[2]);
    ctx[2].uc_stack.ss_sp = stack2;
    ctx[2].uc_stack.ss_size = sizeof(stack2);
    ctx[1].uc_link = &main_ctx;
    makecontext(&ctx[2], func3, 0); 

    printf("swapcontext\n");
    while (count ++ < 30){
        swapcontext(&main_ctx, &ctx[count%3]);
    }  

    printf("\n");

#endif

}