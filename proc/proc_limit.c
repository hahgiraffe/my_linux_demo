#include <sys/resource.h>
#include "tlpi_hdr.h"

void print(int resource)
{
    struct rlimit rlim;
    if(getrlimit(resource,&rlim)==-1)
    {
        printf("getrlimit error\n");
    }
    printf("soft is ");
    
}


int main(int argc, char const *argv[])
{
    
    return 0;
}
