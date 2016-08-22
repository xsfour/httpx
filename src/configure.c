//
// Created by xsfour on 16/8/20.
//

#include "configure.h"

#include <assert.h>
#include <stddef.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>

struct conf_s {
    struct sockaddr_in name;
    uint16_t port;

    int num_workers;
};

conf_t* conf_get()
{
    static conf_t conf;
    static int flag = 0;

    const uint16_t port = 8080;
    
    if (flag) {
        return &conf;
    }

    flag = 1;
    
    memset(&conf.name, 0, sizeof(conf.name));
    conf.name.sin_family = AF_INET;
    conf.name.sin_port = htons(port);
    conf.name.sin_addr.s_addr = htonl(INADDR_ANY);

    conf.num_workers = 4;
    
    return &conf;
}