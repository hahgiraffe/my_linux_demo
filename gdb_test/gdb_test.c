#include <stdio.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"

int add(int a,int b)
{
    return a+b;
}
int main()
{
    pid_t pid;
    int a=2;
    pid=fork();
    switch(pid)
    {
    case -1:
        printf("fork error\n");
    case 0:
        printf("child process\n");
        int b=3;
        a=add(a,b);
        printf("add %d\n",a);
        _exit(0);
    default:
        pid=wait(NULL);
        break;
    
    }
    printf("father process get childpro %d\n",pid);
    int b=3;
    printf("add %d\n",add(a,b));
    exit(0);

}