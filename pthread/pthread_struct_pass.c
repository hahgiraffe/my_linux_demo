#include <pthread.h>
#include "tlpi_hdr.h"

struct S
{
    int a;
    char *str;
}stru;

void *thread_func(void *arg){
    struct S *s=malloc(sizeof(struct S));
    s->a=1;
    s->str="123";
    pthread_exit((void*) s);//这里其实不太好，应该用shared_ptr，因为这个函数结束的时候不知道s指针所指向的内存是否已经被释放
}
int main(){
    pthread_t pt;
    int err;
    struct S *ptr;
    
    err=pthread_create(&pt,NULL,(void*)thread_func,NULL);
    err=pthread_join(pt,(void *)&ptr);
    printf("the result is %s %d\n",ptr->str,ptr->a);
    free(ptr);
    return 0;
}