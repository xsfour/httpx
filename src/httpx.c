//
//  httpx.c
//  httpx
//
//  Created by xsfour on 16/8/18.
//  Copyright © 2016年 xsfour. All rights reserved.
//

#include "events/kqueue.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>


const int BACKLOG = 5;
int NWORKERS = 4;

int* pids = NULL;

int sock_listen(uint16_t* port);
void terminate(int sig);

int main(int argc, char* argv[])
{
    int sockfd;
    int kq;

    int i;

    uint16_t port = 4444;

    pids = (int*)malloc(NWORKERS * sizeof(int));

    // TODO: get 'port' from options

    for (i = 0; i < NWORKERS; ++i) {
        pids[i] = fork();
        if (pids[i] == 0) {
            sockfd = sock_listen(&port);
            event_loop(sockfd, BACKLOG, i + 1);

            close(sockfd);
            printf("#%d exited\n", i + 1);
            exit(0);
        }
        else if (pids[i] == -1) {
            error_die("fork");
        }
    }

    signal(0, terminate);

    for (i = 0; i < NWORKERS; ++i) {
        waitpid(pids[i], NULL, 0);
    }

    free(pids);

    printf("\nHello, world!\n");
    return 0;
}

int sock_listen(uint16_t* port)
{
    int sockfd = 0;
    struct sockaddr_in name;

    const int optval = 1;

    /* socket */
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        error_die("socket");
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        error_die("setsockopt(SO_REUSEADDR)");
    };

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) == -1) {
        error_die("setsockopt(SO_REUSEPORT)");
    };

    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);

    /* bind */
    if (bind(sockfd, (struct sockaddr *)&name, sizeof(name)) < 0) {
        error_die("bind");
    }

    if (*port == 0) {
        socklen_t name_len = sizeof(name);
        if (getsockname(sockfd, (struct sockaddr *)&name, &name_len) == -1) {
            error_die("getsockname");
        }

        *port = ntohs(name.sin_port);
    }

    /* listen */
    if (listen(sockfd, BACKLOG) < 0) {
        error_die("listen");
    }

    fprintf(stdout, "Listening to 0.0.0.0:%d ...\n", *port);

    return sockfd;
}

void terminate(int sig)
{
    int i;

    assert(pids != NULL);
    
    for (i = 0; i < NWORKERS; ++i) {
        kill(pids[i], sig);
    }
}
