#include "server.h"
#include <stdio.h>

int ws_request(struct conn *c) {

    printf("ws request: %s\n", c->rbuffer);
}

int ws_response(struct conn *c) {}
