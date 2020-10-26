#ifndef SERVER_H
#define SERVER_H

#include "network.h"
#include "connection_list.h"

bool server_start(Server* server, char* ip, unsigned int port);
void server_stop(int socket);

#endif // SERVER_H
