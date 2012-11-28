#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "mongoose.h"

extern const char* complete(const char* s);

bool exit_flag = false;

static void *callback(
    enum mg_event event,
    struct mg_connection *conn)
{
    const struct mg_request_info *request_info = mg_get_request_info(conn);

    if (event == MG_NEW_REQUEST)
    {
        const size_t buf_size = 1000;
        char buffer[buf_size];
        buffer[0] = '\0';
        const char* answer = buffer;
        const char* request = (0 == *(request_info->uri) ? "" : request_info->uri + 1);
        const char* content = "text/html";

        answer = complete(request);
        content = "application/json";

        mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s\r\n", content, strlen(answer)+2, answer);

        return (void *)"";  // Mark as processed
    }
    else { return NULL; }
}

int main(void)
{
    struct mg_context *ctx;
    const char *options[] = {"listening_ports", "8888", NULL};

    ctx = mg_start(&callback, NULL, options);
    getchar();  // Wait until user hits "enter"
    while(!exit_flag) { sleep(1); }
    mg_stop(ctx);

    return 0;
}
