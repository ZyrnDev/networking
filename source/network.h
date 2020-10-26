#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CONNECTIONS 10
#define DEFAULT_PORT 10982
#define BOOL_CHECK(arg) assert(arg == true)

typedef struct Connection {
    struct sockaddr_in info;
    struct pollfd      poll;
} Connection;

#endif // NETWORK_H
