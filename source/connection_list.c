#include "connection_list.h"

ConnectionList connection_list_create(int capacity) {
    ConnectionList connectionList;
    connectionList.capacity = capacity;
    connectionList.count    = 0;
    connectionList.server   = (Server){};
    connectionList.clients  = (Client*) calloc(connectionList.capacity, sizeof(Client));
    if (!connectionList.clients) { }

    return connectionList;
}

void connection_list_destroy(ConnectionList* connectionList) {
    connectionList->capacity = 0;
    connectionList->count = 0;
    // connectionList->server = ;
    free(connectionList->clients);
    connectionList->clients = NULL;
}

struct pollfd* connection_list_get_pollfds(ConnectionList* connectionList, struct pollfd* dest, nfds_t* size) {
    if (connectionList == NULL || size == NULL) {
        return NULL;
    }

    dest = (struct pollfd*) realloc(dest, connectionList->count * sizeof(struct pollfd) + 1);
    if (dest == NULL) {
        *size = 0;
        return NULL;
    }

    // printf("Server FD:\t%d\n", connectionList->server.connection.poll.fd);

    dest[0] = connectionList->server.connection.poll;
    for (size_t i = 0; i < connectionList->count; i++) {
        // printf("Client FD:%d\n", connectionList->clients[i].connection.poll.fd);
        dest[i+1] = connectionList->clients[i].connection.poll;
    }

    // printf("Count %zu\n", connectionList->count + 1);

    *size = connectionList->count + 1;
    return dest;
}

bool connection_list_add(ConnectionList* connectionList, Client client) {
    if (connectionList->count >= connectionList->capacity) {
        return false;
    }

    memcpy(&connectionList->clients[connectionList->count], &client, sizeof(client));
    connectionList->count++;

    return true;
}

bool connection_list_disconnect_at(ConnectionList* connectionList, int index) {
    if (index < 0 || index > (int)connectionList->capacity) {
        return false;
    }

    connectionList->clients[index].state = CC_DISCONNECTED;
    shutdown(connectionList->clients[index].connection.poll.fd, SHUT_RDWR);
    close(connectionList->clients[index].connection.poll.fd);

    return false;
}

bool connection_list_disconnect_clear(ConnectionList* connectionList) {
    size_t usedCount = 0;
    for (size_t i = 0; i < connectionList->capacity; i++) {
        if (connectionList->clients[i].state == CC_DISCONNECTED) {
            // shutdown(connectionList->clients[i].connection.poll.fd, SHUT_RD);
            close(connectionList->clients[i].connection.poll.fd);
            continue;
        }

        connectionList->clients[usedCount] = connectionList->clients[i];
        usedCount++;
    }

    connectionList->count = usedCount;

    return true;
}
