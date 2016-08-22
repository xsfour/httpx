//
//  httpx.c
//  httpx
//
//  Created by xsfour on 16/8/18.
//  Copyright © 2016年 xsfour. All rights reserved.
//

#include "kqueue.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <memory.h>


const int BACKLOG = 5;

int sock_listen(uint16_t* port)
{
    int sockfd = 0;
    struct sockaddr_in name;

    int optval;

    /* socket */
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        error_die("socket");
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
            &optval, sizeof(optval)) == -1) {
        error_die("setsockopt(SO_REUSEADDR)");
    };

#ifdef SO_REUSEPORT
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT,
            &optval, sizeof(optval)) == -1) {
        error_die("setsockopt(SO_REUSEPORT)");
    };
#endif

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

    fprintf(stdout, "Server listening to 0.0.0.0:%d...\n", *port);

    return sockfd;
}

int main(int argc, char* argv[])
{
    int sockfd;
    int kq;

    uint16_t port = 4444;

    // TODO: get 'port' from options

    sockfd = sock_listen(&port);
    event_loop(sockfd, BACKLOG);

    printf("Hello, world!");
    return 0;
}
