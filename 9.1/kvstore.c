
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kvstore.h"

#if ENABLE_ARRAY
extern kvs_array_t global_array;
#endif

void *kvs_malloc(size_t size) {

    return malloc(size);
}

void kvs_free(void *ptr) {

    return free(ptr);   
}

const char *command[] = {
    "SET", "GET", "DEL", "MOD" ,"EXIST",
};


enum {
    KVS_CMD_START = 0,
    KVS_CMD_SET = KVS_CMD_START,
    KVS_CMD_GET,
    KVS_CMD_DEL,
    KVS_CMD_MOD,
    KVS_CMD_EXIST,
    KVS_CMD_COUNT,
};

const char *response[] = {

};

int kvs_split_token(char *msg, char *tokens[]) {

    if(msg == NULL || tokens == NULL) return -1;

    int idx = 0;
    char *token = strtok(msg, " ");
    while (token != NULL) {
        tokens[idx] = token;
        idx++;
        token = strtok(NULL, " ");
    }

    return idx;
}

int kvs_filter_protocol(char **tokens, int count, char *response) {

    if (tokens[0] == NULL || response == NULL || count == 0) return -1;

    int cmd = KVS_CMD_START;
    for (cmd = KVS_CMD_START; cmd < KVS_CMD_COUNT; cmd++) {
        if (strcmp(tokens[0], command[cmd]) == 0) {
            break;
        }
    }

    int length = 0;
    int ret = 0;
    char *key = tokens[1];
    char *value = tokens[2];

    switch(cmd) {
        case KVS_CMD_SET:
            ret = kvs_array_set(&global_array, key, value);
            if (ret < 0) {
                length = sprintf(response, "ERROR\r\n");
            } else if (ret == 0) {
                length = sprintf(response, "OK: SET command processed.\r\n");
            } else {
                length = sprintf(response, "EXIST: Key already exists.\r\n");
            }
            
            break;

        case KVS_CMD_GET:
            char *result = kvs_array_get(&global_array, key);
            if (result == NULL) {
                length = sprintf(response, "ERROR: Key not found.\r\n");
            } else {
                length = sprintf(response, "OK: %s\r\n", result);
            }
            break;
 
        case KVS_CMD_DEL:
            ret = kvs_array_del(&global_array, key);
            if (ret < 0) {
                length = sprintf(response, "ERROR\r\n");
            } else if (ret == 0) {
                length = sprintf(response, "OK: DEL command processed.\r\n");
            } else {
                length = sprintf(response, "ERROR: Key not found.\r\n");
            }
            break;
        case KVS_CMD_MOD:
            ret = kvs_array_mod(&global_array, key, value);
            if (ret < 0) {
                length = sprintf(response, "ERROR\r\n");
            } else if (ret == 0) {
                length = sprintf(response, "OK: MOD command processed.\r\n");
            } else {
                length = sprintf(response, "ERROR: Key not found.\r\n");
            }
            break;
        case KVS_CMD_EXIST:
            ret = kvs_array_exist(&global_array, key);
            if (ret == 0) {
                length = sprintf(response, "EXIST.\r\n");
            } else {
                length = sprintf(response, "NO EXIST.\r\n");
            }
            break;
    }

    return length;
    
}


// mas: request message, length: length of request message, response: response message
int kvs_protocol(char *msg, int length, char *response) {

    if (msg == NULL || response == NULL || length <=0) return -1;

    printf("recv: %d, %s\n", length, msg);

    char *tokens[KVS_MAX_TOKENS] = {0};

    int count = kvs_split_token(msg, tokens);

    if (count == -1) return -1;

    //memcpy(response, msg, length);
    return kvs_filter_protocol(tokens, count, response);
}


int init_kvengine(void) {

#if ENABLE_ARRAY
    memset(&global_array, 0, sizeof(kvs_array_t));
    kvs_array_create(&global_array);
    
#endif
    return 0;
}

// 传什么给底层网络，port，kvs_protocol
int main(int argc, char *argv[]) {

    if (argc != 2) return -1;

    int port = atoi(argv[1]);

    init_kvengine();

#if (NETWORK_SELECT == NETWORK_REACTOR)

    reactor_start(port, kvs_protocol);

#elif (NETWORK_SELECT == NETWORK_PROACTOR)

    proactor_start(port, kvs_protocol);

#enif (NETWORK_SELECT == NETWORK_NTYCO)

    ntyco_start(port, kvs_protocol);

#endif
}