#include "../headers/server.h"
#include "../headers/utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main() {

  signal(SIGPIPE, signal_callback_handler);

  // sigaction(SIGPIPE, &(struct sigaction){SIG_IGN}, NULL);

  char *path = "./files/index.html";

  int server = get_listener_socket();

  int pfds_size = 10;

  struct pollfd *pfds = malloc(pfds_size * sizeof(struct pollfd));

  int pfds_count = 0;

  pfds[0].fd = server;
  pfds[0].events = POLL_IN;

  ++pfds_count;

  while (1) {
    int polling = poll(pfds, pfds_count, -1);

    if (polling == -1) {
      perror("poll()");
      exit(1);
    }
    process_clients(server, &pfds, &pfds_count, &pfds_size, path);
  }

  free(pfds);
}
