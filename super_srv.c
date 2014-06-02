#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#include "server.h"
#pragma comment(lib, "ws2_32.lib")  /* WinSock使用的库函数 */



int main(int argc, char **argv){
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2,0), &wsa_data); /* 初始化 WinSock 资源 */
    //web
    SOCKET	web_srv_soc = 0;
    unsigned short web_port = HTTP_DEF_PORT;
    web_srv_soc = socket(AF_INET, SOCK_STREAM, 0); /* 创建 socket */
    if (web_srv_soc == INVALID_SOCKET)
    {
        printf("[Web] socket() Fails, error = %d\n", WSAGetLastError());
        return -1;
    }
    struct sockaddr_in web_serv_addr;   /* 服务器地址  */
    web_serv_addr.sin_family = AF_INET;
    web_serv_addr.sin_port = htons(web_port);
    web_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int result;
    result = bind(web_srv_soc, (struct sockaddr *) &web_serv_addr, sizeof(web_serv_addr));
    if (result == SOCKET_ERROR) /* 绑定失败 */
    {
        closesocket(web_srv_soc);
        printf("[Web Server] Fail to bind, error = %d\n", WSAGetLastError());
        return -1;
    }
    result = listen(web_srv_soc, SOMAXCONN);
    printf("[Web] The server is running ... ...\n");

    //FTP
    SOCKET	ftp_srv_soc = 0;
    unsigned short ftp_port = FTP_DEF_PORT;
    ftp_srv_soc = socket(AF_INET, SOCK_STREAM, 0); /* 创建 socket */
    if (ftp_srv_soc == INVALID_SOCKET)
    {
        printf("[FTP] socket() Fails, error = %d\n", WSAGetLastError());
        return -1;
    }
    struct sockaddr_in ftp_serv_addr;   /* 服务器地址  */
    ftp_serv_addr.sin_family = AF_INET;
    ftp_serv_addr.sin_port = htons(ftp_port);
    ftp_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    result = bind(ftp_srv_soc, (struct sockaddr *) &ftp_serv_addr, sizeof(ftp_serv_addr));
    if (result == SOCKET_ERROR) /* 绑定失败 */
    {
        closesocket(ftp_srv_soc);
        printf("[FTP Server] Fail to bind, error = %d\n", WSAGetLastError());
        return -1;
    }
    result = listen(ftp_srv_soc, SOMAXCONN);
    printf("[FTP] The server is running ... ...\n");

    //UDP FTP
    SOCKET  uconn_sock = 0; /* socket 句柄 */
    uconn_sock = socket(AF_INET, SOCK_DGRAM, 0); /* 创建 socket */

    unsigned short port = UFTP_DEF_PORT;
    /* socket的本地地址 */
    struct sockaddr_in loc_addr;
    loc_addr.sin_family = AF_INET;
    loc_addr.sin_port = htons(port);
    loc_addr.sin_addr.s_addr = INADDR_ANY;

    bind(uconn_sock, (struct sockaddr *)&loc_addr, sizeof(struct sockaddr));
    printf("[UFTP] The server is running ... ...\n");


    fd_set afds;
    FD_ZERO(&afds);
    FD_SET(web_srv_soc, &afds);
    FD_SET(ftp_srv_soc, &afds);
    FD_SET(uconn_sock, &afds);

    fd_set rfds;
    SOCKET web_conn_soc;
    SOCKET ftp_conn_soc;
    while(1){
        FD_ZERO(&rfds);
        memcpy(&rfds, &afds, sizeof(rfds));
        if(select(FD_SETSIZE, &rfds, (fd_set*)0, (fd_set*)0,
                  (struct timeval *)0)== SOCKET_ERROR)
            printf("select error\n");
        if(FD_ISSET(web_srv_soc, &rfds)){
            struct sockaddr_in conn_addr;   /* 客户端地址  */
            int conn_addr_len = sizeof(conn_addr);
            web_conn_soc = accept(web_srv_soc, (struct sockaddr*)&conn_addr,&conn_addr_len);
            if(web_conn_soc == INVALID_SOCKET){
                printf("[Web] Fail to accept, error = %d\n", WSAGetLastError());
                break;
            }
            printf("[Web] Accepted address:[%s], port:[%d]\n",
            inet_ntoa(conn_addr.sin_addr), ntohs(conn_addr.sin_port));
            char recv_buf[BUF_SIZE];
            int recv_len;
            recv_len = recv(web_conn_soc, recv_buf, BUF_SIZE, 0);
            if (recv_len == SOCKET_ERROR) /* 接收失败 */
            {
                closesocket(web_conn_soc);
                printf("[Web] Fail to recv, error = %d\n", WSAGetLastError());
                break;
            }

            recv_buf[recv_len] = 0;

            /* 向客户端发送响应数据 */
            http_send_response(web_conn_soc, recv_buf, recv_len);
            closesocket(web_conn_soc);
        }
        if(FD_ISSET(ftp_srv_soc, &rfds)){
            struct sockaddr_in conn_addr;   /* 客户端地址  */
            int conn_addr_len = sizeof(conn_addr);
            ftp_conn_soc = accept(ftp_srv_soc, (struct sockaddr*)&conn_addr,&conn_addr_len);
            if(ftp_conn_soc == INVALID_SOCKET){
                printf("[FTP] Fail to accept, error = %d\n", WSAGetLastError());
                break;
            }
            printf("[FTP] Accepted address:[%s], port:[%d]\n",
            inet_ntoa(conn_addr.sin_addr), ntohs(conn_addr.sin_port));
            char recv_buf[BUF_SIZE];
            int recv_len;
            recv_len = recv(ftp_conn_soc, recv_buf, BUF_SIZE, 0);
            char temp[1];
            send(ftp_conn_soc, temp, 1, 0);
            if (recv_len == SOCKET_ERROR) /* 接收失败 */
            {
                closesocket(ftp_conn_soc);
                printf("[FTP] Fail to recv, error = %d\n", WSAGetLastError());
                break;
            }

            recv_buf[recv_len] = 0;

            /* 向客户端发送响应数据 */
            ftp_send_response(ftp_conn_soc, recv_buf, recv_len);
            closesocket(ftp_conn_soc);
        }
        if(FD_ISSET(uconn_sock, &rfds)){
            char recv_buf[BUF_SIZE];
            int recv_len;
            struct sockaddr_in   t_addr; /* socket的远端地址 */
            int addr_len = sizeof(struct sockaddr_in);
            recv_len = recvfrom(uconn_sock, recv_buf, BUF_SIZE, 0,
                              (struct sockaddr *)&t_addr, &addr_len);
            if (recv_len == SOCKET_ERROR)
            {
                printf("[UFTP Server] Recv error %d\r\n", WSAGetLastError());
                break;
            }
            recv_buf[recv_len] = 0;
            printf("[UFTP Server] Receive data: \"%s\", from %s\r\n",
            recv_buf, inet_ntoa(t_addr.sin_addr));
            uftp_send_response(uconn_sock, recv_buf, recv_len, &t_addr);
        }

    }

    closesocket(web_srv_soc);
    closesocket(ftp_srv_soc);
    closesocket(uconn_sock);
    WSACleanup();
    printf("[web] The server is stopped.\n");
    printf("[FTP] The server is stopped.\n");
    printf("[UFTP] The server is stopped.\n");
    return 0;
}
