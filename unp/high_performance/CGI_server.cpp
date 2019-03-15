/*
利用半同步/半异步的进程池来实现CGI_server

*/

#include "hsha_process_pool.h"

class cgi_conn
{
public:
    cgi_conn(){ }
    ~cgi_conn(){ }
public:
    void init(int epollfd,int sockfd,const sockaddr_in& client_addr);
    void process();
private:
    static const int BUFFER_SIZE = 1024;
    static int m_epollfd;
    int m_sockfd;
    sockaddr_in m_address;
    char m_buff[BUFFER_SIZE];
    int m_read_idx;//标记缓冲区中已读入客户数据的最后一个字节的下一个位置
};

void cgi_conn::init(int epollfd,int sockfd,const sockaddr_in& client_addr){
    m_epollfd=epollfd;
    m_sockfd=sockfd;
    m_address=client_addr;
    memset(m_buff,'\0',BUFFER_SIZE);
    m_read_idx=0;
}

void cgi_conn::process(){
    int idx=0;
    int ret=-1;
    while(1)
    {
        idx=m_read_idx;
        ret=recv(m_sockfd,m_buff+idx,BUFFER_SIZE-1-idx,0);
        if(ret<0){
            //如果读取操作错误，则关闭客户连接
            if(errno!=EAGAIN){
                removefd(m_epollfd,m_sockfd);
            }
        }
        //如果对方关闭连接，则服务器也关闭
        else if(ret==0){
            removefd(m_epollfd,m_sockfd);
            break;
        }
        else{
            m_read_idx +=ret;
            printf("user content is %s\n",m_buff);
            for(;idx<m_read_idx;idx++){
                //如果遇到字符“\r\n”则可以开始处理客户数据
                if((idx>=1) && (m_buff[idx-1]=='\r') && (m_buff[idx]=='\n')){
                    break;
                }
            }
            if(idx==m_read_idx){//如果没有遇到字符/r/n，则需要读取更多客户数据
                continue;
            }
            m_buff[idx-1]='\0';
            char *file_name =m_buff;
            if(access(file_name,F_OK) ==-1){
                removefd(m_epollfd,m_sockfd);
                break;
            }
            ret=fork();//创建子进程执行cgi程序
            if(ret==-1){
                removefd(m_epollfd,m_sockfd);
                break;
            }
            else if(ret>0){
                //父进程关闭连接
                removefd(m_epollfd,m_sockfd);
                break;
            }
            else{
                //子进程将标准输出定向到m_sockfd，并执行cgi
                close(STDOUT_FILENO);
                dup(m_sockfd);
                execl(m_buff,m_buff,0);
                exit(0);
            }
        }
    }
}


int cgi_conn::m_epollfd=-1;

int main(int argc,char *argv[]){
    int listenfd;
    struct sockaddr_in addr;
    listenfd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&addr,sizeof(addr));
    addr.sin_family=AF_INET;
    inet_pton(AF_INET,argv[1],&addr.sin_addr);
    addr.sin_port=htons(atoi(argv[2]));
    int err=bind(listenfd,(sockaddr *)&addr,sizeof(addr));
    assert(err!=-1);
    err=listen(listenfd,5);
    assert(err!=-1);
    processpool<cgi_conn> *pool=processpool<cgi_conn>::create(listenfd);
    if(pool){
        pool->run();
        delete pool;
    }
    close(listenfd);//这个listenfd由main函数创建，就应该由main函数关闭
    return 0;
}

