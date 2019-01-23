#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#define BD_NO_CHDIR 01
#define BD_NO_CLOSE_FILE 02
#define BD_NO_REOPEN_STD_FDS 04
#define BD_NO_UMASK 010
#define BD_MAX_CLOSE 8192
int becomeDaemon(int flag);


int becomeDaemon(int flags)
{
    int fd;
    switch(fork()){
    case -1: return -1;
    case 0: break;
    default: _exit(0);
    }

    if(setsid()==-1){
        return -1;
    }
    
    switch(fork()){
    case -1: return -1;
    case 0: break;
    default: _exit(0);
    }

    if(!(flags& BD_NO_UMASK))
    {
        umask(0);
    }

    if(!(flags& BD_NO_CHDIR))
    {
        chdir("/");
    }
    if(!(flags& BD_NO_CLOSE_FILE))
    {
        int maxfd=sysconf(_SC_OPEN_MAX);
        if(maxfd==-1){
            maxfd=BD_MAX_CLOSE;

        }
        
        for(fd=0;fd<maxfd;fd++){
            close(fd);
        }
    }

    if(!(flags& BD_NO_REOPEN_STD_FDS)){
        close(STDIN_FILENO);
        fd=open("/dev/null",O_RDWR);
        if(fd!=STDIN_FILENO){
            return -1;
        }
        if(dup2(STDIN_FILENO,STDOUT_FILENO)!=STDOUT_FILENO){
            return -1;
        }
        if(dup2(STDIN_FILENO,STDERR_FILENO)!=STDERR_FILENO){
            return -1;
        }
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    becomeDaemon(0);
    sleep(20);
    return 0;
}
