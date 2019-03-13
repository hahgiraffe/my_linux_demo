#ifndef LST_TIMER
#define LST_TIMER

#include "http_parse.h"
#include <time.h>
#define BUFF_SEIZE 64
class util_timer; //定时器类声明
/*
头文件定义了一个升序定时器双向链表
添加定时器时间复杂度O(n)
删除定时器时间复杂度O(1)
执行定时器任务时间复杂度O(1)

*/




//用户数据结构，客户端的address，socket文件描述符
//读缓存和定时器
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFF_SEIZE];
    util_timer *timer;
};
//定时器类
class util_timer
{
public:
    util_timer():prev(NULL),next(NULL){ }//构造函数
    time_t expire;//任务超时时间（绝对时间）
    void (*cb_func) (client_data*);//函数指针，client_data*为参数，void返回值，回调函数
    client_data *user_data;//定时器处理的客户数据，由定时器的执行者传递给回调函数
    util_timer *prev;//指向前面的定时器
    util_timer *next;//指向后面的定时器
};
//定时器链表，双向且递增，且带有头结点和尾节点
class sort_time_list
{
public:
    sort_time_list():head(NULL),tail(NULL){ }//构造函数
    ~sort_time_list()//析构函数，把链表中每个元素delete
    {
        util_timer *tmp=head;
        while(tmp){
            head=tmp->next;
            delete tmp;
            tmp=head;
        }
    }
    void add_timer(util_timer *timer);//把目标定时器timer添加到链表中
    void adjust_timer(util_timer *timer);//当某个定时任务发生时候，调整对应定时器在链表中的位置
    void del_timer(util_timer *timer);//将目标定时器从链表中删除
    void tick();//SIGALRM信号每次被触发就在其信号处理函数中（若为统一事件源则在主函数中）执行以此tick函数，以处理到期的任务
private:
    void add_timer(util_timer *timer,util_timer* list_head);
    util_timer *head;
    util_timer *tail;
};

void sort_time_list::add_timer(util_timer *timer){
    if(!timer){
        return;
    }
    if(!head){
        head=tail=timer;
        return;
    }
    if(timer->expire < head->expire){//如果给定的定时器超时时间小于头结点的超时时间，则把其放到头结点
        timer->next=head;
        head->prev=timer;
        head=timer;
        return;
    }
    //如果要插入中间则调用私有重载函数，保证升序
    add_timer(timer,head);
}

void sort_time_list::adjust_timer(util_timer *timer){
    if(!timer){
        return;
    }
    util_timer *tmp=timer->next;
    //当定时器在链表尾部，或者定时器的值任然小于下一个定时器的超时值，则不用调整
    if(!tmp || (timer->expire < tmp->expire)){
        return;
    }
    if(timer==head){//如果定时器在头部，则将其取出，再重新插入
        head=head->next;
        head->prev=NULL;
        timer->next=NULL;
        add_timer(timer,head);
    }
    else
    {
        //如果定时器不在头部，则将其取出，插入后面的节点之中
        timer->prev->next=timer->next;
        timer->next->prev=timer->prev;
        add_timer(timer,timer->next);
    }
}

void sort_time_list::del_timer(util_timer *timer){
    if(!timer){
        return;
    }
    if( (timer==head) && (timer==tail) ){//表示链表中只有一个节点
        delete timer;
        head=NULL;
        tail=NULL;
        return;
    }
    if(timer==head){
        head=head->next;
        head->prev=NULL;
        delete timer;
        return;
    }
    if(timer==tail){
        tail=tail->prev;
        tail->next=NULL;
        delete timer;
        return;
    }
    //如果其位于链表中间
    timer->next->prev=timer->prev;
    timer->prev->next=timer->next;
    return;
}

void sort_time_list::tick(){
    if(!head){
        return;
    }
    printf("timer tick\n");
    time_t cur=time(NULL);//得到当前时间
    util_timer *tmp=head;
    //从链表头结点开始依次处理每个定时器，一直到遇到一个尚未到期的定时器
    while(tmp){
        if(tmp->expire>cur){
            break;//没到期
        }
        //到期
        tmp->cb_func(tmp->user_data);//调用回调函数,执行完以后充值链表头结点
        head=tmp->next;
        if(head){
            head->prev=NULL;
        }
        delete tmp;
        tmp=head;
    }
}

void sort_time_list::add_timer(util_timer *timer,util_timer *list_head){//将timer添加到list_head之后的合适位置
    util_timer *prev=list_head;
    util_timer *tmp=prev->next;
    while(tmp){
        if(timer->expire<tmp->expire){
            prev->next=timer;
            timer->next=tmp;
            tmp->prev=timer;
            timer->prev=prev;
            break;
        }
        prev=tmp;
        tmp=tmp->next;
    }
    //如果遍历完仍然未找到超时时间大于目标定时器的超时时间，则将其插入链表尾部
    if(!tmp){
        prev->next=timer;
        timer->prev=prev;
        timer->next=NULL;
        tail=timer;
    }
}


#endif