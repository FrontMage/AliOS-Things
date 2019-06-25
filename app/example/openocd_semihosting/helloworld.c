/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include <aos/kernel.h>

#include "gap_semihost.h"
#include "pmsis.h"

PI_L2 char write_string_w0[] = {'h','e','l','l','o','\n','\0'};

int application_start(int argc, char *argv[])
{
    int count = 0;
    //printf("nano entry here!\r\n");

    gap8_semihost_write0(write_string_w0);
    while(1) {
        //printf("hello world! count %d \r\n", count++);
        //gap8_semihost_write0("Hello semihosted world!\n");
        gap8_semihost_write0(write_string_w0);
        aos_msleep(1000);
    };
}
