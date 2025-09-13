#ifndef _SERVER_H_
#define _SERVER_H_

#define BUFFER_LENGTH 1024

typedef int (*RCALLBACK)(int fd);

struct conn {
    int fd;

    char rbuffer[BUFFER_LENGTH];
    int rlength;

    char wbuffer[BUFFER_LENGTH];
    int wlength;

    RCALLBACK send_callback;

    union {
        RCALLBACK recv_callback;
        RCALLBACK accept_callback;
    } r_action;
};

int http_request(struct conn *c);
int http_response(struct conn *c);

#endif