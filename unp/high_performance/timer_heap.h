#ifndef HEAP_TIMER
#define HEAP_TIMER

#include "http_parse.h"
#include <time.h>
#include <netinet/in.h>
#define BUFFER_SIZE 64
/*
最小时间堆:将所有定时器中超时时间最小的一个定时器的定时值作为心搏时间
这样一旦心搏函数tick被调用，最小的一个定时器一定到期，我们只需要在tick函数中
处理该定时器即可，然后再从剩余定时器中找到超时时间最小的设置下一次心搏时间
用最小堆即可满足要求

...to be continue,..
*/

class heap_timer;

struct client_data{
    heap_timer *timer;
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
};

//定时器类
class heap_timer
{   
public:
    heap_timer(int delay){
        expire =time(NULL)+delay;
    }

public:
    time_t expire;//定时器生效绝对时间
    void (*cb_func)(client_data *);//回调函数
    client_data *user_data;//用户数据
};

//时间堆类
class time_heap
{
public:
    time_heap(int cap);
    time_heap(heap_timer ** init_heap,int size,int capacity);
    ~time_heap();

public:
    void add_timer(heap_timer *timer);
    void del_timer(heap_timer *timer);
    heap_timer *top() const;
    void pop_timer();
    void tick();
    inline bool empty() const {return cur_size ==0;}

private:
    void percolate_down(int hole);
    void resize();
private:
    int capacity;//容量
    int cur_size;//当前数组包含元素个数
    heap_timer **array;//堆数组，一个小顶堆用一个数组表示
};




#endif
