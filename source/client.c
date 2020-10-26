#include "client.h"

bool client_connect(int* client_socket, char* ip, unsigned int port) {
    if ((*client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Could not create client socket: ");
        return false;
    }

    struct sockaddr_in server_info;
    socklen_t server_socket_size = sizeof(server_info);
    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family      = AF_INET;
    server_info.sin_port        = htons(port);
    server_info.sin_addr.s_addr = inet_addr(ip); // do dis in function lad

    if (connect(*client_socket, (struct sockaddr*)&server_info, server_socket_size) < 0) {
        perror("Could not connect to server: ");
        return false;
    }

    return true;
}

void client_disconnect(int client_socket) {
    shutdown(client_socket, SHUT_RDWR);
    close(client_socket);
}

int main(void) {
    int client_socket;
    BOOL_CHECK(client_connect(&client_socket, "127.0.0.1", DEFAULT_PORT));

    char* buffer = (char*) calloc(256, sizeof(char));

    int bytes_read;
    if ((bytes_read = recv(client_socket, buffer, 256, 0)) > -1) {
        printf("%d:%-256s\n", bytes_read, buffer);
    } else {
        perror("Unable to recieve any data.");
    }

    printf("buffer length: %d\n", (int) strlen(buffer));

    free(buffer);
    buffer = NULL;

    client_disconnect(client_socket);

    return 0;
}
