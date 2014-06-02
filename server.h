#ifndef CLW_SERVER
#define CLW_SERVER

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>


/* 定义常量 */
#define FTP_DEF_PORT        21     /* 连接的缺省端口 */
#define HTTP_DEF_PORT       80     /* 连接的缺省端口 */
#define UFTP_DEF_PORT     1025 /* 缺省端口 */
#define BUF_SIZE      1024     /* 缓冲区的大小 */
#define FILENAME_LEN   256     /* 文件名长度 */

//web
char *http_get_type_by_suffix(const char *suffix);
void http_parse_request_cmd(char *buf, int buflen, char *file_name, char *suffix);
int http_send_response(SOCKET soc, char *buf, int buf_len);

//ftp
void ftp_get(SOCKET soc, char* filename);
void ftp_put(SOCKET soc, char *file_name);
int ftp_send_response(SOCKET soc, char *buf, int buf_len);

//uftp
void uftp_get(SOCKET soc, char* filename, struct sockaddr_in * send_addr);
void uftp_put(SOCKET soc, char *file_name, struct sockaddr_in * recv_addr);
int uftp_send_response(SOCKET soc, char *buf, int buf_len, struct sockaddr_in* send_addr);
#endif
