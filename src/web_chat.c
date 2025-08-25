#include "server.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "3495"
#define BACKLOG 10

int get_listener_socket() {
  struct addrinfo hints, *ai;
  int listener;
  int rv;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "getaddrinfo() from listener failed: %s", gai_strerror(rv));
    exit(1);
  }

  struct addrinfo *p;

  p = ai;

  while (p) {
    listener = socket(p->ai_family, p->ai_socktype, 0);

    if (listener < 0) {
      p = p->ai_next;
      continue;
    }

    if (bind(listener, p->ai_addr, p->ai_addrlen)) {
      close(listener);
      p = p->ai_next;
      continue;
    }

    break;
  }

  if (p == NULL) {
    return -1;
  }

  freeaddrinfo(ai);

  if (listen(listener, BACKLOG)) {
    return -1;
  }

  return listener;
}

void add_pfd(int fd, struct pollfd **pfds, int *pfds_count, int *pfds_size) {
  if (*pfds_count == *pfds_size)
    *pfds = realloc(*pfds, *pfds_count * 2 * sizeof(struct pollfd));

  (*pfds)[*pfds_count].fd = fd;
  (*pfds)[*pfds_count].events = POLLIN;
  (*pfds)[*pfds_count].revents = 0;

  (*pfds_count)++;
}

void delete_pfd(int i, struct pollfd **pfds, int *pfds_count) {
  (*pfds)[i] = (*pfds)[*pfds_count - 1];

  (*pfds_count)--;
}

void handle_new_connection(int listener, struct pollfd **pfds, int *pfds_count,
                           int *pfds_size) {

  struct sockaddr_storage addr;
  socklen_t addr_len;

  int client = accept(listener, (struct sockaddr *)&addr, &addr_len);

  if (client < 0) {
    perror("accept() failed");
  }

  add_pfd(client, pfds, pfds_count, pfds_size);
}

void handle_client_data(int listener, struct pollfd **pfds, int *pfds_count,
                        int fd_index) {

  char buffer[100];

  int sender_fd = (*pfds)[fd_index].fd;

  int bytes_received = recv(sender_fd, buffer, sizeof buffer, 0);

  if (bytes_received <= 0) {
    if (bytes_received == 0) {
      printf("socket %d hung up\n", sender_fd);
    } else {
      perror("recv call failed");
    }

    close(sender_fd);
    delete_pfd(fd_index, pfds, pfds_count);
  } else {
    // bytes_received, send something back

    for (int i = 0; i < *pfds_count; i++) {
      int dest_fd = (*pfds)[i].fd;

      if (dest_fd != listener && dest_fd != sender_fd) {
        if (send(dest_fd, buffer, bytes_received, 0)) {
          perror("send call failed");
        }
        printf("sending...");
      }
    }
  }
}

void process_connections(int listener, struct pollfd **pfds, int *pfds_count,
                         int *pfds_size) {

  for (int i = 0; i < *pfds_count; i++) {
    if ((*pfds)[i].revents & (POLLIN | POLL_HUP)) {

      if ((*pfds)[i].fd == listener) {
        handle_new_connection(listener, pfds, pfds_count, pfds_size);
      } else {
        handle_client_data(listener, pfds, pfds_count, i);
      }
    }
  }
}

// TODO

int main() {
  int pfds_size = 5;
  int pfds_count = 0;
  struct pollfd *pfds;

  pfds = malloc(pfds_size * sizeof(struct pollfd));

  int listener = get_listener_socket();

  pfds[0].fd = listener;
  pfds[0].events = POLLIN;

  pfds_count = 1;

  while (1) {
    int polling = poll(pfds, pfds_count, -1);

    if (polling < 0) {
      perror("poll");
      exit(1);
    }

    process_connections(listener, &pfds, &pfds_count, &pfds_size);
  }

  free(pfds);
}
