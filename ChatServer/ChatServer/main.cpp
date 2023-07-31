#include <cstdio>
#include <map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cerrno>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/wait.h>

const int port = 9527;
using namespace std;
map<int, string> clnt_sock_maps;
int clnt_num = 0;


void updateUserInfo() { //向所有客户端更新用户名和用户列表

    char message[1024] = "#2";
    string userList = "";
    for (map<int, string>::iterator it = clnt_sock_maps.begin(); it != clnt_sock_maps.end(); it++) {
        userList.append(it->second);
        userList.append("\n");
    }
    strcat(message, userList.c_str());
    for (map<int, string>::iterator it = clnt_sock_maps.begin(); it != clnt_sock_maps.end(); it++) {
        write(it->first, message, sizeof(message));
        printf("#2write to %s : %s\n", it->second.c_str(), message);
    }

    memset(message, 0, sizeof(message));
    message[0] = '#';
    message[1] = '3';
    sprintf(message + 2, "%d ", clnt_num);
    for (map<int, string>::iterator it = clnt_sock_maps.begin(); it != clnt_sock_maps.end(); it++) {

        write(it->first, message, sizeof(message));
        printf("#2write to %s : %s\n", it->second.c_str(), message);

    }
}



void server() {

    int serv_sock, clnt_sock;
    sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_sz;
    char buf[1025] = "";
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(port);

    if (bind(serv_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        printf("bind()error , %d , %s \n", errno, strerror(errno));
        close(serv_sock);
        return;
    }
    if (listen(serv_sock, 5) == -1) {
        printf("listen()error , %d , %s \n", errno, strerror(errno));
        close(serv_sock);
        return;
    }

    int epfd, event_cnt = 0;
    epfd = epoll_create(100);
    if (epfd == -1) {
        printf("epoll_create()error , %d , %s \n", errno, strerror(errno));
        close(serv_sock);
        return;
    }
    epoll_event event;
    epoll_event* all_events = new epoll_event[100];
    event.events = EPOLLIN;
    event.data.fd = serv_sock;

    epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);
    printf("Server Start!\n");
    while (true) {
        event_cnt = epoll_wait(epfd, all_events, 100, 1000);
        if (event_cnt == -1) {
            printf("epoll_wait()error , %d , %s \n", errno, strerror(errno));
            break;
        }
        if (event_cnt == 0) continue;
        //printf("-*-event_cnt : %d -*-\n", event_cnt);
        for (int i = 0; i < event_cnt; i++) {
            clnt_sock = all_events[i].data.fd;

            if (clnt_sock == serv_sock) {
                clnt_sz = sizeof(clnt_adr);
                clnt_sock = accept(serv_sock, (sockaddr*)&clnt_adr, &clnt_sz);
                event.events = EPOLLIN;
                event.data.fd = clnt_sock;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
                clnt_sock_maps.insert(pair<int, string>(clnt_sock, "null"));
                printf("client %d is connect! \n", clnt_sock);
                clnt_num++;
            }
            else {
                memset(buf, 0, sizeof(buf));
                ssize_t len = read(clnt_sock, buf, sizeof(buf));
                if (buf[0] == '\n') continue;
                if (len <= 0) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, clnt_sock, NULL);
                    close(clnt_sock);
                    clnt_sock_maps.erase(clnt_sock);
                    printf("client %d is disconnect! \n", clnt_sock);
                    clnt_num--;
                    updateUserInfo();
                }
                else {
                    printf("client %d send %s\n", clnt_sock, buf);
                    if (buf == "\n")continue;
                    if (buf[0] == '#') {
                        if (buf[1] == '1') {    //#1设置用户名
                            char name[128] = "";
                            strcat(name, buf + 2);
                            //printf("%s\n++", name);
                            map<int, string>::iterator it = clnt_sock_maps.find(clnt_sock);
                            it->second = name;
                        }
                        updateUserInfo();
                        continue;
                    }
                    char message[1026] = "";
                    map<int, string>::iterator it = clnt_sock_maps.find(clnt_sock);
                    strcat(message, it->second.c_str());
                    strcat(message, ": ");
                    strcat(message, buf);
                    for (map<int, string>::iterator it = clnt_sock_maps.begin(); it != clnt_sock_maps.end(); it++){
                        write(it->first, message, sizeof(message));

                        printf("write to %s : %s\n", it->second.c_str(), message);
                    }

                }
            }
        }
    }
    delete[]all_events;
}

void client() {
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_adr;
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_adr.sin_port = htons(port);  //htonl : long ，四个字节  ；htons  short

    if (connect(sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        printf("connect()error , %d , %s \n", errno, strerror(errno));
        close(sock);
        return;
    }
    char message[1026] = "";
    //1.创建epoll实例
    int epfd = epoll_create(1);

    //2.添加要监听的事件
    struct epoll_event  evt;//自带的结构体
    evt.events = EPOLLIN;//设置事件类型为读事件， 触发方式为水平触发
    evt.data.fd = sock;//事件可以带一个数据（监听套接字）
    int  ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &evt);
    if (ret < 0)
        perror("add error");

    //3.等待事件发生
    epoll_event* evtArray = new  epoll_event[100];//定义存储发生的事件的数组

    pid_t pid = fork();

    while (true) {
        if (pid == 0) {
            int event_cnt = epoll_wait(epfd, evtArray, 10, 1000);
            if (event_cnt == -1) {
                printf("epoll_wait()error , %d , %s \n", errno, strerror(errno));
                break;
            }
            if (event_cnt == 0) continue;

            for (int i = 0; i < event_cnt; i++) {
                memset(message, 0, strlen(message));
                int len = read(evtArray[i].data.fd, message, sizeof(message));
                if (len > 0)
                    printf("--%s\n", message);
            }
        }
        else {
            printf("Input message(q to quit): \n");
            fgets(message, sizeof(message), stdin);
            if (!strcmp(message, "q\n") || !strcmp(message, "Q\n")) {
                kill(pid, SIGKILL);
                close(sock);
                break;
            } 
            write(sock, message, strlen(message));
        }
    }
    close(sock);
}



int main(int argc, char* argv[])
{
    if (strcmp(argv[1], "s") == 0) {
        server();
    }
    else if (strcmp(argv[1], "c") == 0) {
        client();
    }
    else {
        printf("%s\n", argv[1]);
        printf("启动失败，请输入正确的命令！\n");
    }
    return 0;
}