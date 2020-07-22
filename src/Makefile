CFLAGS = -Wall -g
CC = cc
SOURCELIST = list.c client.c topic.c

build_server:
	$(CC) $(CFLAGS) $(SOURCELIST) server.c -o server
build_subscriber:
	$(CC) $(CFLAGS) subscriber.c -o subscriber
run_server:
	./server

run_client:
	./client

clean:
	rm -f server client
