#ifndef TIME_WHELL_TIMER
#define TIME_WHELL_TIMER

#include "http_parse.h"
#include <time.h>
#include <netinet/in.h>
/*
    高性能定时器之时间轮
    时间轮采用hash的思想，将定时器散列在不同的槽中
    这样每个槽中的定时器个数小于链表定时器的数目，插入操作即可简化

*/
#define BUFF_SIZE 64
class tw_timer;

struct client_data{
    tw_timer *timer;
    sockaddr_in address;
    int sockfd;
    char buf[BUFF_SIZE];
};

class tw_timer
{
public:
    tw_timer(int rot,int ts):prev(NULL),next(NULL),rotation(rot),time_slot(ts){ }

public:
    int rotation;//记录定时器在时间轮中转多少圈后生效
    int time_slot;//记录定时器在哪个槽
    void (*cb_func)(client_data *);//回调函数
    client_data *user_data;//用户数据
    tw_timer *prev;
    tw_timer *next;
};

class time_wheel
{
public:
    tw_timer *add_timer(int timeout);
    void delete_timer(tw_timer *timer);
    void tick();

private:
    static const int N =60;//时间轮上的槽数
    static const int SI = 1;//每隔一秒时间轮转一次，即槽间隙为1
    tw_timer *slot[N];//每个槽中有一个定时器链表
    int cur_slot;//时间轮当前槽
};

//根据定时值timeout新建一个定时器，并加到链表中
tw_timer * time_wheel::add_timer(int timeout){
    if(timeout<0){
        return;
    }
    int ticks=0;
    if(timeout < SI){
        ticks=1;
    }
    else{
        ticks= timeout/SI;
    }
    int rotation=ticks/N;//多少圈后触发
    int ts=(cur_slot + (ticks % N)) %N;//最终加入的槽
    tw_timer *timer=new tw_timer(rotation,ts);
    if(!slot[ts]){
        //当前ts槽中无任何定时器
        printf("add timer ,rotation is %d , ts is %d ,cur_slot is %d\n",rotation,ts,cur_slot);
        slot[ts]=timer;
    }
    else{
        timer->next=slot[ts];
        slot[ts]->prev=timer;
        slot[ts]=timer;
    }
    return timer;
}

void time_wheel::delete_timer(tw_timer *timer){
    if(!timer){
        return;
    }
    int ts=timer->time_slot;
    if(timer==slot[ts]){
        //如果节点在该槽中头节点，则需要重置头结点
        slot[ts]=slot[ts]->next;
        if(slot[ts]){
            slot[ts]->prev=NULL;
        }
        delete timer;
        
    }
    else
    {
        timer->prev->next=timer->next;
        if(timer->next){
            timer->next->prev=timer->prev;
        }
        delete timer;
    } 

}

//SI时间到后，调用该函数，时间轮向前滚动一个槽的间隔
void time_wheel::tick(){
    tw_timer *tmp=slot[cur_slot];
    printf("current slot is %d\n",cur_slot);
    while(tmp){
        printf("tick once\n");
        if(tmp->rotation>0){
            //如果该定时器rotation大于0，则这一轮不起作用
            tmp->rotation--;
            tmp=tmp->next;
        }
        else{
            //次定时器到期，执行定时任务并删除该定时器
            tmp->cb_func(tmp->user_data);
            if(tmp==slot[cur_slot]){
                printf("delete head of cur_slot\n");
                slot[cur_slot]=tmp->next;
                delete tmp;
                if(slot[cur_slot]){
                    slot[cur_slot]->prev=NULL;
                }
                tmp=slot[cur_slot];//往后移动一个
            }
            else{
                tmp->prev->next=tmp->next;
                if(tmp->next){
                    tmp->next->prev=tmp->prev;
                }
                tw_timer *tmp2=tmp->next;
                delete tmp;
                tmp=tmp2;
            }
        }
    }
    cur_slot= (++cur_slot)%N;
}


#endif