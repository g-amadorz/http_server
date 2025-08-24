CC = gcc
CFLAGS = -Wall -Wextra -g

.PHONY: server tcp_server_fork

server: server.c
	$(CC) $(CFLAGS) server.c -o server

tcp_server_fork: tcp_server_fork.c
	$(CC) $(CFLAGS) tcp_server_fork.c -o tcp_server_fork
