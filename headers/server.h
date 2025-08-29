#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>

int get_listener_socket();
void process_connections();
void receive_client(int listener_fd, struct pollfd **pfds, int *pfds_count,
                    int *pfds_size);
void handle_client_connection();
void drop_client();
