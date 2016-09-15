//
// Created by xsfour on 16/8/22.
//

#ifndef HTTPX_KQUEUE_H
#define HTTPX_KQUEUE_H

typedef struct event_s event_t;

int event_loop(int sockfd, int backlog);

#endif //HTTPX_KQUEUE_H
