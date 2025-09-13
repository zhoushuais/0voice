#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "server.h"

int http_request(struct conn *c) {

    printf("request: %s\n", c->rbuffer);

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

#else

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

    close(filefd);


#endif

    return c->wlength;

}

