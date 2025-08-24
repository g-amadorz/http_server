#include "server.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "3495"
#define BACKLOG 10

int main() {
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, PORT, &hints, &res)) {
    fprintf(stderr, "getaddrinfo() call failed: (%d)\n", errno);
    return 2;
  }

  int socket_listen =
      socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if (bind(socket_listen, res->ai_addr, res->ai_addrlen)) {
    fprintf(stderr, "bind() call failed: (%d)\n", errno);
    return 2;
  }

  if (listen(socket_listen, BACKLOG)) {
    fprintf(stderr, "listen() call failed: (%d)\n", errno);
  }

  printf("Waiting for connections...\n");

  while (1) {
    struct sockaddr_storage client_address;
    socklen_t addr_size = sizeof client_address;
    int socket_client =
        accept(socket_listen, (struct sockaddr *)&client_address, &addr_size);

    char address_buffer[100];
    getnameinfo((struct sockaddr *)&client_address, addr_size, address_buffer,
                sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    int pid = fork();

    if (pid == 0) {
      while (1) {
        char read[1024];

        int bytes_received = recv(socket_client, read, strlen(read), 0);

        if (bytes_received < 1) {
          close(socket_client);
          exit(0);
        }

        for (int j = 0; j < bytes_received; j++) {
          read[j] = toupper(read[j]);
        }

        send(socket_client, read, strlen(read), 0);
      }
    }

    close(socket_client);
  }

  printf("Closing listening socket...\n");

  close(socket_listen);
}
