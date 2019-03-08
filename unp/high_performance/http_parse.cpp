#include "http_parse.h"
using namespace std;
#define BUFFER_SIZE 4096
/*
    这个主要是http的解析过程

*/


//主状态机的两种状态，当前正在分析请求行和正在分析头部字段
enum CHECK_STATE{CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER};
//从状态机的三种可能状态，即行的读取状态：读取到一个完整的行、行出错和行数据暂且不完整
enum LINE_STATUS{LINE_OK = 0, LINE_BAD, LINE_OPEN};

//服务器处理HTTP请求的结果：NO_REQUEST表示请求不完整，需要读取客户数据;
//                          GET_REQUEST表示获得了一个完整的客户请求;
//                          BAD_REQUEST表示客户请求有语法错误;
//                          FORBIDDEN_REQUEST表示客户对资源没有足够的访问权限
//                          INTERNAL_ERROR表示服务器内部错误;
//                          CLOSED_CONNECTION表示客户端已经关闭连接。
enum HTTP_CODE{NO_REQUEST, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};

static const char* szret[] = {"I get a correct result\n", "Something wrong\n"};

//从状态机，用于解析一行内容
LINE_STATUS parse_line(char* buffer, int &checked_index, int &read_index) {
    //checked_id_index指向buffer的正在分析的字节，read_index指向buffer中的最后一个字节的下一个字节
    //即从0~checked_index是已分析完毕，checked_index~read_index-1待分析
    char temp;
    for(; checked_index < read_index; ++ checked_index) {
        temp = buffer[checked_index];
        //如果当前是回车符，则说明可能读取到了一个完整行
        //如果是'\n'，即换行符，也说明可能读取到了一个完整行
        if(temp == '\r') {
            //如果当前是本行最后一个字符，则说明不完整，需要更多数据
            //如果下一个字符是'\n'则说明读取到了完整的行
            //否则说明HTTP请求存在语法问题
            if(checked_index + 1 == read_index) {
                return LINE_OPEN;
            }
            else if(buffer[checked_index + 1] == '\n') {
                buffer[checked_index ++] = '\0';
                buffer[checked_index ++] = '\0';
                return LINE_OK;
            }
            else return LINE_BAD;
        }
        else if(temp == '\n') {
            if((checked_index > 1) && (buffer[checked_index - 1] == '\r')) {
                buffer[checked_index - 1] = '\0';
                buffer[checked_index ++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    //如果到最后也没有发现'\r'字符，则返回LINE_OPEN表示需要读取更多数据分析
    return LINE_OPEN;
}

//分析请求行
HTTP_CODE parse_requestline(char* temp, CHECK_STATE& checkstate) {
    //如果请求行中没有空格和'\t'字符则说明HTTP请求有问题
    //strpbrk返回前面缓冲区第一个在后面字符集合中的字符位置
    char* url = strpbrk(temp, " \t");
    if(!url) return BAD_REQUEST;
    *url ++ = '\0';

    //strcasecmp与strcmp的区别就是不区分大小写
    char* method = temp;
    if(strcasecmp(method, "GET") == 0) printf("The request method is GET\n");
    else return BAD_REQUEST;

    //strspn函数统计缓冲区前面多少个连续字符在字符集合中
    url += strspn(url, "\t");
    char *version = strpbrk(url, " \t");
    if(!version) return BAD_REQUEST;

    *version ++ = '\0';
    version += strspn(version, " \t");

    //strchr函数返回缓冲区里第一个后面字符的位置
    if(strcasecmp(version, "HTTP/1.1") != 0) {
        url += 7;
        url = strchr(url, '/');
    }

    if(!url || url[0] != '/') return BAD_REQUEST;
    printf("The request URL is: %s\n", url);
    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//分析头部
HTTP_CODE parse_headers(char* temp) {
    //遇到空行说明得到了一个正确的HTTP请求
    if(temp[0] == '\0') return GET_REQUEST;
    else if(strncasecmp(temp, "Host:", 5) == 0) {
        temp += 5;
        temp += strspn(temp, " \t");
        printf("The request host is: %s\n", temp);
    }
    else printf("I can not handle this header\n");
    return NO_REQUEST;
}

//分析HTTP请求的入口函数
HTTP_CODE parse_content(char* buffer, int& checked_index, CHECK_STATE& checkstate, int& read_index, int &start_line) {
    LINE_STATUS linestatus = LINE_OK;
    HTTP_CODE retcode = NO_REQUEST;
    while((linestatus = parse_line(buffer, checked_index, read_index)) == LINE_OK) {
        char* temp = buffer + start_line;
        start_line = checked_index;
        switch(checkstate) {
            case CHECK_STATE_REQUESTLINE: {
                retcode = parse_requestline(temp, checkstate);
                if(retcode == BAD_REQUEST) return BAD_REQUEST;
                break;
            }
            case CHECK_STATE_HEADER: {
                retcode = parse_headers(temp);
                if(retcode == BAD_REQUEST) return BAD_REQUEST;
                else if(retcode == GET_REQUEST) return GET_REQUEST;
                break;
            }
            default: {
                return INTERNAL_ERROR;
            }
        }
    }
    if(linestatus == LINE_OPEN) return NO_REQUEST;
    else return BAD_REQUEST;
}

int main(int argc, char** argv) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, ip, &address.sin_addr);

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int reuse = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct sockaddr_in client_address;
    socklen_t client_addrlength = sizeof(client_address);
    int fd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
    if(fd < 0) printf("errno is: %d\n", errno);
    else {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));
        //下面的变量分别代表已经接收的字符数、已经读取了多少字节、已经分析完了多少字节、行在buffer中的起始位置
        int data_read = 0;
        int read_index = 0;
        int checked_index = 0;
        int start_line = 0;
        CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
        while(1) {
            data_read = recv(fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
            if(data_read == -1) {
                printf("reading failed\n");
                break;
            }
            else if(data_read == 0) {
                printf("remote client has closed the connection\n");
                break;
            }
            read_index += data_read;
            HTTP_CODE result = parse_content(buffer, checked_index, checkstate, read_index, start_line);
            if(result == NO_REQUEST) continue;
            else if(result == GET_REQUEST) {
                send(fd, szret[0], strlen(szret[0]), 0);
                break;
            }
            else {
                send(fd, szret[1], strlen(szret[1]), 0);
                break;
            }
        }
        close(fd);
    }
    close(listenfd);
    return 0;
}