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

http_response build_response(char *path, int client_fd);

const char *get_content_type(const char *path);
