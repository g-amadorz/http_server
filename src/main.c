#include "../headers/server.h"
#include <signal.h>

int main() {

  char *path = "./files/index.html";

  int server = get_listener_socket();

  int pfds_size = 10;

  struct pollfd *pfds = malloc(pfds_size * sizeof(struct pollfd));

  int pfds_count = 0;

  pfds[0].fd = server;
  pfds[0].events = POLL_IN;

  while (1) {
    int polling = poll(pfds, pfds_count, -1);

    if (polling == -1) {
      exit(1);
    }
    process_clients(server, &pfds, &pfds_count, &pfds_size, path);
  }
}
