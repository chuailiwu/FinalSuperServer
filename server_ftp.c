#include "server.h"
void ftp_get(SOCKET soc, char* filename){

    FILE *file;
    file = fopen(filename, "rb+");
    if(file == NULL){
        printf("[FTP] The file [%s] is not existed\n", filename);
        exit(1);
    }

    int file_len;
    fseek(file, 0, SEEK_END);
    file_len = ftell(file);
    fseek(file, 0, SEEK_SET);

    int read_len;
    char read_buf[BUF_SIZE];
    do /* 发送文件文件*/
    {
        read_len = fread(read_buf, sizeof(char), BUF_SIZE, file);

        if (read_len > 0)
        {
            send(soc, read_buf, read_len, 0);
            file_len -= read_len;
        }
    } while ((read_len > 0) && (file_len > 0));

    fclose(file);

}

void ftp_put(SOCKET soc, char *file_name){

    FILE *file_ftp;
    file_ftp = fopen(file_name, "w+");
    if(file_ftp == NULL){
        printf("[FTP] The file [%s] is not existed\n", file_name);
        exit(1);
    }

    int result = 0;
    char data_buf[BUF_SIZE];
    do /* 接收响应并保存到文件中 */
    {
        result = recv(soc, data_buf, BUF_SIZE, 0);
        if (result > 0)
        {
            fwrite(data_buf, sizeof(char), result, file_ftp);

            /* 在屏幕上输出 */
            data_buf[result] = 0;
            printf("%s", data_buf);
        }
    } while(result > 0);

    fclose(file_ftp);

}
/**

*/
int ftp_send_response(SOCKET soc, char *buf, int buf_len){

    buf[buf_len] = 0;
    printf("The Server received: '%s' cmd from client \n", buf);
    //get filename
    char file_name[FILENAME_LEN];
    strcpy(file_name, buf+4);
    //文件下载
    if (strncmp(buf,"get",3)==0)  ftp_get(soc, file_name);
    //文件上传
    else if (strncmp(buf,"put",3)==0)  ftp_put(soc, file_name);
    else printf("the commod is not found\n");
    return 0;
}
