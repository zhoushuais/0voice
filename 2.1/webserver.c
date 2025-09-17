#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "server.h"
#include <string.h>
#include <sys/sendfile.h>
#include <errno.h>

int http_request(struct conn *c) {

    printf("request: %s\n", c->rbuffer);

    memset(c->wbuffer, 0, BUFFER_LENGTH);
    c->wlength = 0;

    c->status = 0;

}

int http_response(struct conn *c) {

#if 0
    c->wlength = sprintf(c->wbuffer, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 82\r\n"
        "Date: Fri, 12 Sep 2025 11:52:52 GMT\r\n"
        "<html><head><title>Hello, World!</title></head><body><h1>Hello, World!</h1></body></html>\r\n"
    );

#elif 0

    int filefd = open("index.html", O_RDONLY);

    struct stat stat_buf;
    fstat(filefd, &stat_buf);


    c->wlength = sprintf(c->wbuffer, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: %ld\r\n"
        "Date: Fri, 12 Sep 2025 11:52:52 GMT\r\n",
        stat_buf.st_size
    );

    int count = read(filefd, c->wbuffer + c->wlength, BUFFER_LENGTH - c->wlength);
    c->wlength += count;

    close(filefd);

#elif 0

    int filefd = open("index.html", O_RDONLY);

    struct stat stat_buf;
    fstat(filefd, &stat_buf);

    if (c->status == 0) {
        c->wlength = sprintf(c->wbuffer, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: %ld\r\n"
            "Date: Fri, 12 Sep 2025 11:52:52 GMT\r\n",
            stat_buf.st_size
        );
        c->status = 1;
    } else if (c->status == 1) {
        int ret =sendfile(c->fd, filefd, NULL, stat_buf.st_size);
        if (ret == -1) {
            printf("errno: %d\n", errno);
        }
        c->wlength = 0;
        memset(c->wbuffer, 0, BUFFER_LENGTH);
        c->status = 2;
    } else if (c->status ==2) {
        c->wlength = 0;
        memset(c->wbuffer, 0, BUFFER_LENGTH);
        c->status = 0;
    }

    close(filefd);

#else
    int filefd = open("vip.png", O_RDONLY);

    struct stat stat_buf;      // struct stat 包含了文件的详细信息
    fstat(filefd, &stat_buf);   // 通过文件描述符获取文件信息

    if (c->status == 0) {
        c->wlength = sprintf(c->wbuffer,       // 格式化的数据写入字符串str: 目标字符串缓冲区,format: 格式化字符串（包含占位符）,...: 可变参数（要格式化的数据）
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: image/png\r\n"
            "Accept-Ranges: bytes\r\n"
            "Content-Length: %ld\r\n"
            "Date: Fri, 12 Sep 2025 11:52:52 GMT\r\n",
            stat_buf.st_size
        );
        c->status = 1;
    } else if (c->status == 1) {
        int ret =sendfile(c->fd, filefd, NULL, stat_buf.st_size);  // 在两个文件描述符之间直接传输数据
        if (ret == -1) {
            printf("errno: %d\n", errno);
        }
        c->wlength = 0;
        memset(c->wbuffer, 0, BUFFER_LENGTH);
        c->status = 2;
    } else if (c->status ==2) {
        c->wlength = 0;
        memset(c->wbuffer, 0, BUFFER_LENGTH);
        c->status = 0;
    }

    close(filefd);

#endif

    return c->wlength;

}

