#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "tlpi_hdr.h"
int main(int argc, char const *argv[])
{
    if(argc!=4)
    {
        printf("argc not correct\n");
    }
    int which=(argv[1][0]=='p')?PRIO_PROCESS:(argv[1][0]=='g')?PRIO_PGRP:PRIO_USER;
    id_t who=atol(argv[2]);
    int prio=atoi(argv[3]);
    prio=getpriority(which,who);
    if(prio==-1 && errno!=0){
        printf("error getpriority\n");
    }
    printf("Nice value is %d\n",prio);
    
    return 0;
}
