CC=clang
CFLAGS=-W -std=c99 -g -lpthread
LDFLAGS=-lSDL2main -lSDL2

server: source/connection_list.c source/server.c
	$(CC) $(CFLAGS) source/connection_list.c source/server.c -o ./build/server

client: source/client.c
	$(CC) $(CFLAGS) source/client.c -o ./build/client

all: server client
