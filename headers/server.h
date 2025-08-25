#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int get_listener_socket();
void process_connections();
void handle_client_connection();
