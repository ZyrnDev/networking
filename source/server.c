#include "server.h"

size_t stupid = 0;

bool server_start(Server* server, char* ip, unsigned int port) {
    if (server == NULL) {
        fprintf(stderr, "Must pass in a valid non-null \'socket_fd\'!");
        return false;
    }

    if ((server->connection.poll.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Could not create server socket: ");
        return false;
    }

    fcntl(server->connection.poll.fd, F_SETFL, O_NONBLOCK);


    memset((struct sockaddr_in*) &server->connection.info, 0, sizeof(server->connection.info));

    socklen_t server_socket_size = sizeof(struct sockaddr_in);

    server->connection.info.sin_family      = AF_INET;
    server->connection.info.sin_port        = htons(port);
    server->connection.info.sin_addr.s_addr = inet_addr(ip);

    int reuse = 1;
    setsockopt(server->connection.poll.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

    if (bind(server->connection.poll.fd, (struct sockaddr*) &server->connection.info, server_socket_size) < 0) {
        perror("Could not bind server socket: ");
        return false;
    }

    if (listen(server->connection.poll.fd, MAX_CONNECTIONS) < 0) {
        perror("Failed to listen on server socket: ");
        return false;
    }

    server->connection.poll.events = POLLIN;
    server->running = true;
    return true;
}

void server_stop(int socket) {
    shutdown(socket, SHUT_RDWR);
    close(socket);
}

/******************************************************************************************************/

bool server_accept_connection(int server_socket, Client* client) {
    if (client == NULL) {
        return false;
    }
    client->state = CC_AWAITING_CONNECTION;

    struct sockaddr_in client_info;
    int client_socket;
    socklen_t client_socket_size = sizeof(struct sockaddr_in);

    client->connection.poll.fd = accept(server_socket, (struct sockaddr*) &client->connection.info, &client_socket_size);
    client->connection.poll.events = POLLIN | POLLOUT;

    if (client->connection.poll.fd < 0) {
        perror("Failed to accept connection....");
        client->state = CC_DISCONNECTED;
        return false;
    }

    client->state = CC_CONNECTED;

    return true;
}

bool server_message_send(int socket, char* message) {
    if (message == NULL) {
        return NULL;
    }

    int bytes_sent = send(socket, message, strlen(message) + 1, 0);

    if (bytes_sent < 0) {
        perror("Could not send data to client: ");
        return false;
    } else {
        printf("Sending %d bytes to client (%d)\n", bytes_sent, socket);
    }

    return true;
}

void console_client_message(char* message) {
    printf("Client\t::> %s\n", message);
}

void console_server_message(char* message) {
    printf("Server\t::> %s\n", message);
}

void console_read_command(char* buffer) {
    printf("Console\t::> ");
    scanf("%1023s", buffer);
}

// maybe not temp: function for existing server on user input
void* server_console_thread(void* arg) {
    Server* server = (Server*) arg;
    if (server == NULL) {
        return NULL;
    }

    console_server_message("Server Started....");

    char input[1024];
    console_read_command(input);
    while (server->running) {
        if (strcmp(input, "stop") == 0) {
            console_server_message("Server Stopping...");
            server->running = false;
            break;
        } else if (strcmp(input, "test") == 0) {
            console_server_message("dum idot");
        } else {
            console_server_message("Command not found");
        }

        console_read_command(input);
    }

    return NULL;
}

int main(void) {
    ConnectionList connections = connection_list_create(MAX_CONNECTIONS); // includes server
    connections.server.running = false;
    BOOL_CHECK(server_start(&connections.server, "0.0.0.0", DEFAULT_PORT));

    pthread_t input_thread;
    pthread_create(&input_thread, NULL, server_console_thread, &connections.server);

    nfds_t size_but_fixed = 0;
    struct pollfd* connection_fds = connection_list_get_pollfds(&connections, NULL, &size_but_fixed);

    //temp 
    struct pollfd poll_fds[MAX_CONNECTIONS + 1] = {};
    nfds_t fd_count = 1;

    poll_fds[0].fd = connections.server.connection.poll.fd;
    poll_fds[0].events = POLLIN;

    while (connections.server.running) {
        int poll_count = poll(connection_fds, size_but_fixed, 1000);

        if (poll_count == -1) {
            perror("Failed to poll for events: ");
            continue;
        }

        // better polish people but on the network
        for (size_t i = 0; i < size_but_fixed; i++) {
            Connection* current = (i == 0) ? (&connections.server.connection) : (&connections.clients[i-1].connection);
            if (connection_fds[i].revents & POLLIN) {
                if (connection_fds[i].fd == connections.server.connection.poll.fd) {
                    Client client;
                    if (server_accept_connection(connections.server.connection.poll.fd, &client)) {
                        if (!connection_list_add(&connections, client)) {
                            printf("Server has reached max capacity %d of %d...\n", (int)connections.count, MAX_CONNECTIONS);
                        } else {
                            printf("Client Connected: %d\n", client.connection.poll.fd);  
                            connection_fds = connection_list_get_pollfds(&connections, connection_fds, &size_but_fixed);
                        }
                    } else {
                        fprintf(stderr, "Unable to accept the connection.\n");
                    }
                } else {
                    char buf[256];
                    int num_bytes = recv(connection_fds[i].fd, buf, sizeof(buf), 0);

                    if (num_bytes <= 0) {
                        // Got error or connection closed by client
                        if (num_bytes == 0) {
                            // Connection closed
                            printf("Client hung up (%s)\n", inet_ntoa(current->info.sin_addr));
                        } else {
                            perror("Recieve data from client failed: ");
                        }

                        current->poll.events = 0;
                        connection_fds[i].events = 0;

                        connection_list_disconnect_at(&connections, i - 1);
                        connection_list_disconnect_clear(&connections);
                    }
                }
            } else if (connection_fds[i].revents & POLLOUT) {
                char* message = "You are ready to have a message...";
                char* buff = (char*) calloc(100, sizeof(char));
                int a = strlen(message);
                strcpy(buff, message);
                buff[a] = '0' + (stupid % 10);
                buff[a+1] = '\0';
                stupid++;
                server_message_send(connection_fds[i].fd, buff);
                free(buff);
                buff = NULL;
            }
        }
    }

    server_stop(connections.server.connection.poll.fd);

    free(connection_fds);
    connection_fds = NULL;

    connection_list_destroy(&connections);

    return 0;
}
