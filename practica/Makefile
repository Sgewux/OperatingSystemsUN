CC=gcc
CFLAGS=-Wall -O2

all: server client create_index

server: server.c utils.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c utils.h
	$(CC) $(CFLAGS) -o client client.c

create_index: create_index.c utils.h
	$(CC) $(CFLAGS) -o create_index create_index.c

clean:
	rm -f server client create_index

