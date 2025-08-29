
// SO Far going to only implemenet get requests
#include <stddef.h>
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
