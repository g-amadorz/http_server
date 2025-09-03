#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

typedef struct http_response http_response;

struct http_request {
  char *method;
  char *version;
  char **headers;
};

struct http_response {
  int status_code;
  char *version;
  char *status_message;
  char *content_type;
  char *body;
  size_t body_length;
};

const char *get_content_type(const char *path);

void send_200_response(int client);
void send_400_response(int client);
void send_404_response(int client);
