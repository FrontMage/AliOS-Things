/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include "pmsis.h"
#include "gap_semihost.h"
#include <aos/kernel.h>

PI_L2 char write_string[] = {'h','e','l','l','o',' ','f','r','i','e','n','d','s','\n','\0'};

int application_start(int argc, char *argv[])
{
    int count = 0;
    printf("nano entry here!\r\n");
    //gap8_semihost_write0(write_string);
    while(1) {
        printf("hello world! count %d \r\n", count++);
	//gap8_semihost_write0(write_string);

        aos_msleep(1000);
    };
}
