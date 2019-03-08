#include <unistd.h>
#include <stdio.h>

int main(){
    __uid_t uid=getuid();
    __uid_t euid=geteuid();
    printf("uid is %d,euid is %d\n",uid,euid);
    return 0;
}
