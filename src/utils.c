//
// Created by xsfour on 16/8/22.
//

#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

void error_die(const char* mark)
{
    perror(mark);
    exit(1);
}
