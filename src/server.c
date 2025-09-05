#include "../headers/server.h"
#include "../headers/http.h"
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT "80"
#define BACKLOG 10
#define P_SIZE 1024

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

void send_400_response(int client) {
  const char *response = "HTTP/1.1 400 Bad Request\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 11\r\n"
                         "Connection: close\r\n"
                         "\r\n"
                         "Bad Request";

  send(client, response, strlen(response), 0);
}

void send_404_response(int client) {
  const char *payload = "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/plain"
                        "Content-Length: 9\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "Not Found";
  send(client, payload, strlen(payload), 0);
}

void send_200_response(int client, FILE *file, const char *content_type) {
  char payload[P_SIZE];

  if (fseek(file, 0L, SEEK_END) != 0) {
    perror("fseek()");
    exit(1);
  }

  size_t file_size = ftell(file);

  rewind(file);

  sprintf(payload, "HTTP/1.1 200 OK\r\n");

  send(client, payload, strlen(payload), 0);

  sprintf(payload, "Content-Type: %s\r\n", content_type);

  send(client, payload, strlen(payload), 0);

  sprintf(payload, "Content-Length: %zu\r\n", file_size);

  send(client, payload, strlen(payload), 0);

  sprintf(payload, "Connection: close\r\n");

  send(client, payload, strlen(payload), 0);

  int r = fread(payload, 1, P_SIZE, file);
  while (r) {
    send(client, payload, r, 0);
    r = fread(payload, 1, P_SIZE, file);
  }

  fclose(file);
}

void handle_client_request(int server, int client_i, struct pollfd **pfds,
                           int *pfds_count, char *resource_path) {

  int client = (*pfds)[client_i].fd;

  if (!strstr(resource_path, "/")) {
    fprintf(stderr, "error no such file");
    send_400_response(client);
    drop_client(client_i, pfds, pfds_count);
  }

  FILE *file = fopen(resource_path, "r");

  if (!file) {
    perror("File opening failed");
    send_404_response(client);
    drop_client(client_i, pfds, pfds_count);
  }

  const char *content_type = get_content_type(resource_path);
  send_200_response(client, file, content_type);
}

void process_clients(int server, struct pollfd **pfds, int *pfds_count,
                     int *pfds_size, char *resource_path) {
  // Poll for new connections
  for (int i = 0; i < *pfds_count; i++) {

    if ((*pfds)[i].revents & POLLIN) {
      if ((*pfds)[i].fd == server) {
        receive_client(server, pfds, pfds_count, pfds_size);
      } else {
        handle_client_request(server, i, pfds, pfds_count, resource_path);
      }
    }
  }
}
