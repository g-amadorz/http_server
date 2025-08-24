#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT "8080"
#define BACKLOG 10

int main() {
  struct sockaddr_storage their_addr;
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int socketfd;
  int new_fd;
  int status;
  int bytes_sent;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if ((status = bind(socketfd, res->ai_addr, res->ai_addrlen)) != 0) {
    fprintf(stderr, "bind: %s\n", gai_strerror(status));
    return 2;
  }

  listen(socketfd, BACKLOG);

  addr_size = sizeof their_addr;
  new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);

  char *msg = "Morris Rocks!";

  bytes_sent = send(new_fd, msg, strlen(msg), 0);
}
