#define _GNU_SOURCE
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#if 1

typedef ssize_t (*read_t)(int fd, void *buf, size_t count);
read_t read_f;

typedef ssize_t (*write_t)(int fd, const void *buf, size_t count);
write_t write_f;

ssize_t read(int fd, void *buf, size_t count) {
    
    struct pollfd fds[1] = {0};
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    int res = poll(fds, 1, 0);
    if (res < 0) {
        swapcontext();

    } else {
        
        ssize_t ret = read_f(fd, buf, count);
        printf("read: %s\n", (char *)buf);
        return ret;
    }
}

ssize_t write(int fd, const void *buf, size_t count) {

    printf("write: %s\n", (const char *)buf);
    return write_f(fd, buf, count);
}

void Init_hook(void) {
    if (!read_f) {
        read_f = dlsym(RTLD_NEXT, "read");
    }

    if (!write_f) {
        write_f = dlsym(RTLD_NEXT, "write");
    }

}

#endif

int main() {

    Init_hook();

    int fd = open("a.txt", O_CREAT | O_WRONLY);
    if (fd < 0) {
        return -1;
    }

    char *str = "1234567890";
    int ret = write(fd, str, strlen(str));
    printf("ret: %d\n", ret);

    close(fd);

    fd = open("a.txt", O_CREAT | );
    if (fd < 0) {
        return -1;
    }

    char buffer[128] = {0};
    ret = read(fd, buffer, 128);
    printf("ret: %d, buffer: %s\n", ret,buffer);

    return 0;
}