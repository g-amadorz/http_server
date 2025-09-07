#include <arpa/inet.h>
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

#define PORT "8080"
#define BACKLOG 10
#define P_SIZE 1024

const char *inet_ntop2(void *addr, char *buf, size_t size) {
  struct sockaddr_storage *sas = addr;
  struct sockaddr_in *sa4;
  struct sockaddr_in6 *sa6;
  void *src;

  switch (sas->ss_family) {
  case AF_INET:
    sa4 = addr;
    src = &(sa4->sin_addr);
    break;
  case AF_INET6:
    sa6 = addr;
    src = &(sa6->sin6_addr);
    break;
  default:
    return NULL;
  }

  return inet_ntop(sas->ss_family, src, buf, size);
}

const char *get_content_type(const char *path) {
  const char *last_dot = strrchr(path, '.');
  if (last_dot) {
    if (strcmp(last_dot, ".css") == 0)
      return "text/css";
    if (strcmp(last_dot, ".csv") == 0)
      return "text/csv";
    if (strcmp(last_dot, ".gif") == 0)
      return "image/gif";
    if (strcmp(last_dot, ".htm") == 0)
      return "text/html";
    if (strcmp(last_dot, ".html") == 0)
      return "text/html";
    if (strcmp(last_dot, ".ico") == 0)
      return "image/x-icon";
    if (strcmp(last_dot, ".jpeg") == 0)
      return "image/jpeg";
    if (strcmp(last_dot, ".jpg") == 0)
      return "image/jpeg";
    if (strcmp(last_dot, ".js") == 0)
      return "application/javascript";
    if (strcmp(last_dot, ".json") == 0)
      return "application/json";
    if (strcmp(last_dot, ".png") == 0)
      return "image/png";
    if (strcmp(last_dot, ".pdf") == 0)
      return "application/pdf";
    if (strcmp(last_dot, ".svg") == 0)
      return "image/svg+xml";
    if (strcmp(last_dot, ".txt") == 0)
      return "text/plain";
  }

  return "application/octet-stream";
}

void add_pfd(int fd, struct pollfd **pfds, int *pfds_count, int *pfds_size) {
  if (*pfds_count == *pfds_size) {
    *pfds = realloc(*pfds, 2 * (*pfds_count) * sizeof(struct pollfd));
    *pfds_size *= 2;
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
    perror("getaddrinfo()");
    exit(1);
  }

  temp = ai;

  while (temp) {

    listener_fd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);

    if (listener_fd < 0) {
      temp = temp->ai_next;
      continue;
    }

    if (bind(listener_fd, temp->ai_addr, temp->ai_addrlen) < 0) {
      close(listener_fd);
      temp = temp->ai_next;
      continue;
    }

    break;
  }

  if (temp == NULL) {
    printf("No socket available\n");
    exit(1);
  }

  if (listen(listener_fd, BACKLOG)) {
    perror("listen()");
    exit(1);
  }

  freeaddrinfo(ai);

  return listener_fd;
}

void receive_client(int listener_fd, struct pollfd **pfds, int *pfds_count,
                    int *pfds_size) {

  struct sockaddr_storage addr;
  char remoteIP[INET6_ADDRSTRLEN];

  socklen_t sock_size = sizeof addr;

  int client = accept(listener_fd, (struct sockaddr *)&addr, &sock_size);

  if (client < 0) {
    perror("client refused connection");
    exit(1);
  }

  add_pfd(client, pfds, pfds_count, pfds_size);
  printf("pollserver: new connection from %s on socket %d\n",
         inet_ntop2(&addr, remoteIP, sizeof remoteIP), client);
}

void drop_client(int client_i, struct pollfd **pfds, int *pfds_count) {
  close((*pfds)[client_i].fd);
  drop_pfd(client_i, pfds, pfds_count);
}

void send_400_response(int client) {
  const char *payload = "HTTP/1.1 400 Bad Request\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 11\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "Bad Request";

  if (send(client, payload, strlen(payload), 0) <= 0) {
    perror("send failed in 400 response");
  }
}

void send_404_response(int client) {
  const char *payload = "HTTP/1.1 404 Not Found\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 9\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "Not Found";

  if (send(client, payload, strlen(payload), 0) <= 0) {
    perror("send failed in 404 response");
  }
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

  sprintf(payload, "\r\n");

  send(client, payload, strlen(payload), 0);

  int r = fread(payload, 1, P_SIZE, file);
  while (r) {
    send(client, payload, r, 0);
    r = fread(payload, 1, P_SIZE, file);
  }
}

void handle_client_request(int server, int client_i, struct pollfd **pfds,
                           int *pfds_count, char *resource_path) {

  int client = (*pfds)[client_i].fd;

  if (!strstr(resource_path, "/")) {
    fprintf(stderr, "Invalid path format: %s\n", resource_path);
    perror("Error no such file");
    send_400_response(client);
    drop_client(client_i, pfds, pfds_count);
    return;
  }

  FILE *file = fopen(resource_path, "rb");

  if (!file) {
    perror("fopen");
    fprintf(stderr, "Failed to open file: %s\n", resource_path);
    send_404_response(client);
    drop_client(client_i, pfds, pfds_count);
    return;
  }

  const char *content_type = get_content_type(resource_path);
  send_200_response(client, file, content_type);
  fclose(file);
}

void process_clients(int server, struct pollfd **pfds, int *pfds_count,
                     int *pfds_size, char *resource_path) {
  // Poll for new connections
  for (int i = 0; i < *pfds_count; i++) {

    if ((*pfds)[i].revents & (POLLIN | POLLHUP)) {
      if ((*pfds)[i].fd == server) {
        receive_client(server, pfds, pfds_count, pfds_size);
      } else {
        handle_client_request(server, i, pfds, pfds_count, resource_path);
      }
    }
  }
}
