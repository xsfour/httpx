//
// Created by xsfour on 16/8/22.
//

#include "kqueue.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <memory.h>
#include <errno.h>

static int loop_flag = 1;

static void print_time(const char* msg);
static int print_buff(const char* buf);

static inline int event_wait(int kq, struct kevent events[],
                      int nevents_max, struct timespec* ts);
static inline int event_ctrl(int kq,
                             int ident, int16_t filter,
                             uint16_t flags, uint32_t fflags,
                             intptr_t data, void* udata);

int event_loop(int sockfd, int backlog)
{
    const int MAX_EVENTS = 1024;

    int kq;
    struct kevent events[MAX_EVENTS];

    struct timespec ts;

    int nevents;
    int clientfd;
    int ident;
    int flags;

    char buf[1024];
    ssize_t nrecv;

    int cnt = 0;

    assert(sockfd > 0);
    printf("Sockfd = %d\n", sockfd);

    if ((kq = kqueue()) == -1) {
        error_die("kqueue");
    };

    if (event_ctrl(kq, sockfd, EVFILT_READ,
            EV_ADD | EV_ENABLE, 0, backlog, 0) < 0) {
        error_die("kevent(delete)");
    };

    ts.tv_sec = 0;
    ts.tv_nsec = 100000;
    while (loop_flag) {
        ++cnt;

        nevents = event_wait(kq, events, MAX_EVENTS, &ts);

        for (int i = 0; i < nevents; ++i) {
            printf("(%d/%d@%d) \n", i + 1, nevents, cnt);

            ident = (int)events[i].ident;
            flags = events[i].flags;

            if (flags & EV_ERROR) {
                fprintf(stderr, "Error occurred(ident=%du, f=%x): %s\n",
                        ident, flags, strerror((int)events[i].data));
                exit(1);
            }
            else if (flags & EV_EOF) {
                printf("Client %du closed\n", ident);
                close(ident);
            }
            else if (ident == sockfd) {
                if ((clientfd = accept(sockfd, NULL, NULL)) < 0) {
                    error_die("accept");
                };

                printf("New client connected:%du\n", clientfd);
                if (event_ctrl(kq, clientfd, EVFILT_READ,
                        EV_ADD | EV_ENABLE, 0, 0, 0) < 0) {
                    error_die("kevent(add)");
                };
            }
            else if (events[i].filter & EVFILT_READ) {
                // TODO: 调用 HTTP 模块处理

                memset(buf, 0, sizeof(buf));
                if ((nrecv = recv(ident, buf, 1024, 0)) < 0) {
                    printf("Descriptor:%du, flags=%x\n", ident, flags);
                    error_die("recv");
                };

                printf("Received %3ld chars from client %du:\n", nrecv, ident);

                if (print_buff(buf)) {
                    if (send(ident, "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n<h1>Hello, Chrome</h1>\r\n",
                            sizeof("HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n<h1>Hello, Chrome</h1>\r\n"), 0) < 0) {
                        error_die("send");
                    };
                }
            }
        }
    }

    return 0;
}

int event_wait(int kq, struct kevent events[],
                      int nevents_max, struct timespec* ts)
{
    int nevents;

    nevents = kevent(kq, NULL, 0, events, nevents_max, ts);
    if (nevents < 0) {
        error_die("event_wait");
    }

    return nevents;
}

int event_ctrl(int kq,
               int ident, int16_t filter,
               uint16_t flags, uint32_t fflags,
               intptr_t data, void* udata)
{
    struct kevent ev;

    EV_SET(&ev, ident, filter, flags, 0, data, udata);

    return kevent(kq, &ev, 1, NULL, 0, NULL);
}

void print_time(const char* msg)
{
    struct timeval tv;
    int h, m, s;

    gettimeofday(&tv, NULL);

    h = tv.tv_sec / 3600;
    m = (tv.tv_sec % 3600) / 60;
    s = tv.tv_sec % 60;

    printf("%d:%d:%d %ld us", h, m, s, (long)tv.tv_usec);
    if (msg != NULL) {
        printf(" (%s)", msg);
    }
    printf("\n");
}

int print_buff(const char* buf)
{
    int i = 0;
    char c;

    int state = 0;

    assert(NULL != buf);

    while ((c = buf[i++])) {
        switch (c) {
            case '\r':
                if (state == 0 || state == 2) {
                    ++state;
                }

//                fputs("\\r", stdout);
                break;
            case '\n':
                if (state == 1 || state == 3) {
                    ++state;
                }

                fputs("↙\n", stdout);
                break;
            default:
                state = 0;

                fputc(c, stdout);
                break;
        }
    }

    return state == 4;
}