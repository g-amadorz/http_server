#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>

int main(void) {

  struct pollfd pfds[1];

  pfds[0].fd = 0;
  pfds[0].events = POLLIN;
}
