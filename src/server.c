#include "../headers/server.h"
#include "../headers/http.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "80"
#define BACKLOG 10

void handle_client_connection() { return; }

void receive_client() {}

void drop_client() {}

int get_listener_socket() {
  struct addrinfo hints, *ai, *temp;
  int listener_fd;
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, PORT, &hints, &ai)) {
    fprintf(stderr, "getaddrinfo() from listener failed");
    exit(1);
  }

  for (temp = ai; temp != NULL; temp = temp->ai_next) {
    listener_fd = socket(temp->ai_family, temp->ai_socktype, 0);
    if (listener_fd < 0) {
      continue;
    }

    if (bind(listener_fd, temp->ai_addr, temp->ai_addrlen)) {
      continue;
    }

    break;
  }

  if (temp == NULL) {
    fprintf(stderr, "No socket available");
    exit(1);
  }

  if (listen(listener_fd, BACKLOG)) {
    fprintf(stderr, "Socket unable to listen");
    exit(1);
  }

  freeaddrinfo(ai);
  freeaddrinfo(temp);

  return listener_fd;
}

int main(int argc, char *argv[]) { return EXIT_SUCCESS; }
