#ifndef CLIENT_H
#define CLIENT_H

#include "network.h"

bool client_connect(int* client_socket, char* ip, unsigned int port);
void client_disconnect(int client_socket);

#endif // CLIENT_H