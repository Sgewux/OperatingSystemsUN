CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: collector agent viewer

collector: collector.c
	$(CC) $(CFLAGS) -o collector collector.c

agent: agent.c
	$(CC) $(CFLAGS) -o agent agent.c

viewer: viewer.c
	$(CC) $(CFLAGS) -o viewer viewer.c

clean:
	rm -f collector agent viewer
