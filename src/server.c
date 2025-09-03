// #include "../headers/server.h"
#include "../headers/http.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "80"
#define BACKLOG 10

void add_pfd(int fd, struct pollfd **pfds, int *pfds_count, int *pfds_size) {
  if (*pfds_count == *pfds_size) {
    pfds = realloc(*pfds, 2 * (*pfds_count) * sizeof(struct pollfd));
  }

  (*pfds)[*pfds_count].fd = fd;
  (*pfds)[*pfds_count].events = POLLIN;
  (*pfds)[*pfds_count].revents = 0;

  (*pfds_count)++;
}

void drop_pfd(int fd_index, struct pollfd **pfds, int *pfds_count) {
  (*pfds)[fd_index] = (*pfds)[*pfds_count - 1];
  (*pfds_count)--;
}

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
    fprintf(stderr, "No socket available", errno);
    exit(1);
  }

  if (listen(listener_fd, BACKLOG)) {
    fprintf(stderr, "Socket unable to listen", errno);
    exit(1);
  }

  freeaddrinfo(ai);
  freeaddrinfo(temp);

  return listener_fd;
}

void receive_client(int listener_fd, struct pollfd **pfds, int *pfds_count,
                    int *pfds_size) {

  struct sockaddr_storage *addr;
  socklen_t sock_size;

  int client = accept(listener_fd, (struct sockaddr *)addr, &sock_size);

  if (client < 0) {
    perror("client refused connection");
    exit(1);
  }

  add_pfd(client, pfds, pfds_count, pfds_size);
}

void drop_client(int client_i, struct pollfd **pfds, int *pfds_count) {
  close((*pfds)[client_i].fd);
  drop_pfd(client_i, pfds, pfds_count);
}

void handle_client_request(int listener_fd, int client_fd,
                           char *resource_path) {

  if (!strstr(resource_path, "/")) {
    fprintf(stderr, "error no such file");

    send_400_response();
  }

  FILE *file = fopen(resource_path, "r");

  if (!file) {
    perror("File opening failed");
    send_404_response();
  }

  send_200_response();
}
void process_clients(int listener_fd, struct pollfd **pfds, int *pfds_count,
                     int *pfds_size, char *resource_path) {

  // Poll for new connections
  for (int i = 0; i < *pfds_count; i++) {

    if ((*pfds)[i].revents & POLLIN) {
      if ((*pfds)[i].fd == listener_fd) {
        receive_client(listener_fd, pfds, pfds_count, pfds_size);
      } else {
        handle_client_request(listener_fd, (*pfds)[i].fd, resource_path);
      }
    }
  }
}

int main() {

  char *path = "./files/index.html";

  int listener_fd = get_listener_socket();

  int pfds_size = 10;

  struct pollfd *pfds = malloc(pfds_size * sizeof(struct pollfd));

  int pfds_count = 0;

  add_pfd(listener_fd, &pfds, &pfds_count, &pfds_size);

  while (1) {
    int polling = poll(pfds, pfds_count, -1);

    if (polling == -1) {
      perror("poll() failed");
      exit(1);
    }

    process_clients(listener_fd, &pfds, &pfds_count, &pfds_size, path);
  }
}
