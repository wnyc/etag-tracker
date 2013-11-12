#include<inttypes.h>
#include <stdio.h>
#include <alloca.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "mongoose.h"

// This function will be called by mongoose on every new request.
static int event_handler(struct mg_event *event) {
  
  if (event->type == MG_REQUEST_BEGIN) {
    if (strcmp(event->request_info->request_method, "GET")) 
      goto gimmie_a_404; 
    if (strcmp(event->request_info->uri, "/session_id"))
      goto gimmie_a_404;
    for (int header=0; header < event->request_info->num_headers && header < 64; header++) {
      if (!strcmp(event->request_info->http_headers[header].name, 
		  "If-None-Match"))
	goto gimmie_a_304;
    }
  
    char *content=alloca(1024);
    
    int fd = open("/dev/urandom", O_RDONLY);
    if (!fd) {
      printf("Boohiss\n");
      goto boohiss;
    }
    uint64_t etag;
    int bytes;
    if ((bytes=read(fd, &etag, sizeof(etag))) != sizeof(etag)) {
      printf("%d bytes\n", bytes);
      goto boohiss;
    }
    close(fd);
    
      // Prepare the message we're going to send
    int content_length = snprintf(content, 1024,
				  "{\"id\": \"%0"PRIx64"16\"}",
				  etag);
    // Send HTTP reply to the client
    mg_printf(event->conn,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
	"Content-Length: %d\r\n"        // Always set Content-Length
	"Etag: %llx\r\n"
        "\r\n"
	      "%s",
	      content_length, etag, content);

    // Returning non-zero tells mongoose that our function has replied to
    // the client, and mongoose should not send client any more data.
    return 1;

  boohiss: ;
    content = "Something seems wrong with our random number generator.";
    mg_printf(event->conn,
        "HTTP/1.1 503 Error\r\n"
        "Content-Type: application/json\r\n"
	"Content-Length: %lud\r\n"        // Always set Content-Length
        "\r\n"
	      "%s",
	      strlen(content), content);
    return 1;
  gimmie_a_304: ;

    mg_printf(event->conn,
        "HTTP/1.1 304 Not Modified\r\n"
        "\r\n"
	      );    
    return 1;
  gimmie_a_404: ;
    content = "<h1>404 Lost</h1><img src=\"lost.jpg\">"
    mg_printf(event->conn,
        "HTTP/1.1 503 Error\r\n"
        "Content-Type: application/json\r\n"
	"Content-Length: %lud\r\n"        // Always set Content-Length
        "\r\n"
	      "%s",
	      strlen(content), content);

    return 1;
  }

  // We do not handle any other event
  return 0;
}

int main(void) {
  struct mg_context *ctx;

  // List of options. Last element must be NULL.
  const char *options[] = {"listening_ports", "8080", NULL};

  // Start the web server.
  ctx = mg_start(options, &event_handler, NULL);
  while(1) sleep(60);
  return 0;
}
