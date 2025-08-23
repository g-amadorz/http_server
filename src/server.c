#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main() {
  struct addrinfo hints, *res;
  int socketfd;
  int status;

  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo("google.com", "8080", &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }

  socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  if ((status = connect(socketfd, res->ai_addr, res->ai_addrlen)) != 0) {
    fprintf(stderr, "connect: %s\n", gai_strerror(status));
  }
}
