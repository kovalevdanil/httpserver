#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

#include <stdio.h>
#include <string.h>

#include <malloc.h>

#include "thread_pool.h"
#include "pages_control.h"
#include "codes.h"

pages_t *pages = NULL;

struct response
{
    uint16_t code;
    page_t *page;
};

typedef struct response response_t;

char *get_url(char *request)
{
    char *search = strstr(request, "GET");

    if (search == NULL)
        return NULL;

    char *page = malloc(256);
    sscanf(search, "GET %s HTTP/", page);

    return page;
}

char *construct_reponse(response_t *response) // it needs to be freed
{
    if (response == NULL)
        return NULL;

    char *response_text = calloc(4096, sizeof(char));

    snprintf(response_text, 4096, RESPOND, response->code, response->page->content_len, response->page->content);

    return response_text;
}

struct worker_args
{
    int16_t fd;
    uint8_t close_connection;    // 0 or 1
    uint8_t free_args_memory;    // 0 or 1
    uint8_t free_request_memory; // 0 or 1
    char *request;
};

typedef struct worker_args worker_args_t;

void postprocess_worker_args(worker_args_t *wargs)
{
    if (wargs->free_request_memory == 1)
        free(wargs->request);

    if (wargs->close_connection == 1)
        close(wargs->fd);

    if (wargs->free_args_memory == 1)
        free(wargs);
}

// worker takes worker_args and processes the request
void worker(void *args)
{
    worker_args_t *wargs = args;

    int fd = wargs->fd;
    char *request_text = wargs->request;
    char *request_url = get_url(request_text);

    if (request_url == NULL) // 400 bad request
    {
        postprocess_worker_args(wargs);
        return;
    }

    response_t response;
    response.code = OK;
    response.page = pages_get_by_url(pages, request_url);

    if (response.page == NULL)
    {
        send(fd, RESPOND_BAD_REQUEST, strlen(RESPOND_BAD_REQUEST) * sizeof(char), 0);
        postprocess_worker_args(wargs);
        return;
    }

    char *response_text = construct_reponse(&response);

    send(fd, response_text, strlen(response_text) * sizeof(char), 0);

    free(response_text);
    postprocess_worker_args(wargs);
}

int start_listen()
{
#define PORT 8080
#define BACKLOG 10

    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1)
        return -1;

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // get rid of "port already in use"
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        return -1;

    if (listen(sockfd, BACKLOG) == -1)
        return -1;

    return sockfd;
}

void start_server()
{
#define BUFFER_SIZE 4096
#define MAX_RESPOND_SIZE 4096
#define POOL_SIZE 20

    int sockfd = start_listen();

    if (sockfd == -1)
    {
        perror("Unable to create listening socket\n");
        return;
    }

    tpool_t *tpool = tpool_create(POOL_SIZE);

    char buffer[BUFFER_SIZE + 1] = {0};
    char respond[MAX_RESPOND_SIZE] = {0};
    char ip[INET_ADDRSTRLEN];

    int new_sock = 0;
    struct sockaddr_storage storage;
    int storage_len = 0;

    while (true)
    {
        new_sock = accept(sockfd, (struct sockaddr *)&storage, &storage_len);

        inet_ntop(AF_INET, ((struct sockaddr *)&storage), ip, INET_ADDRSTRLEN);
        printf("New connection on socket %d: %s\nl", new_sock, ip);

        int recv_size = recv(new_sock, buffer, BUFFER_SIZE, 0);

        printf("Recevied: %d bytes\n", recv_size);

        if (recv_size == 0)
        {
            close(new_sock);
            fprintf(stderr, "Connection lost: %s\n", ip);
            continue;
        }

        buffer[recv_size] = '\0';
        printf("%s", buffer);

        worker_args_t *args = calloc(1, sizeof(worker_args_t));
        args->fd = new_sock;
        args->close_connection = 1;
        args->free_args_memory = 1;
        args->free_request_memory = 1;
        args->request = calloc(recv_size + 1, sizeof(char));
        memcpy(args->request, buffer, recv_size);
        tpool_add_work(tpool, worker, args);
    }
}

int main()
{
    pages = pages_create();
    pages_init(pages, NULL);
    start_server();
}