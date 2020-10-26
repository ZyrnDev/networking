#ifndef CONNECTION_LIST
#define CONNECTION_LIST

#include "network.h"

typedef enum {
    CC_DISCONNECTED,
    CC_CONNECTED,
    CC_AWAITING_CONNECTION
} ClientConnection;

typedef struct Client {
    Connection       connection;
    ClientConnection state;
} Client;

typedef struct Server {
    Connection connection;
    bool       running;
} Server;

typedef struct ConnectionList {
    size_t  capacity;
    size_t  count;
    Server  server;
    Client* clients;
} ConnectionList;

ConnectionList connection_list_create(int capacity);
void connection_list_destroy(ConnectionList* connectionList);

struct pollfd* connection_list_get_pollfds(ConnectionList* connectionList, struct pollfd* dest, nfds_t* size);

bool connection_list_add(ConnectionList* connectionList, Client client);
bool connection_list_disconnect_at(ConnectionList* connectionList, int index);
bool connection_list_disconnect_clear(ConnectionList* connectionList);

#endif // CONNECTION_LIST
