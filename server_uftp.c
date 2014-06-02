#include "server.h"
void uftp_get(SOCKET soc, char* filename, struct sockaddr_in * send_addr){

    FILE *file;
    file = fopen(filename, "rb+");
    if(file == NULL){
        printf("[UFTP] The file [%s] is not existed\n", filename);
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
            sendto(soc, read_buf, read_len, 0,
               (struct sockaddr *)send_addr, sizeof(struct sockaddr));
            file_len -= read_len;
        }
    } while ((read_len > 0) && (file_len > 0));
    sendto(soc, read_buf, 0, 0,
               (struct sockaddr *)send_addr, sizeof(struct sockaddr));

    fclose(file);

}

void uftp_put(SOCKET soc, char *file_name, struct sockaddr_in * recv_addr){

    FILE *file_ftp;
    file_ftp = fopen(file_name, "w+");
    if(file_ftp == NULL){
        printf("[FTP] The file [%s] is not existed\n", file_name);
        exit(1);
    }

    int result = 0;

    char data_buf[BUF_SIZE]={0};
    int addr_len = sizeof(struct sockaddr);
 /*
    char file_len[BUFSIZ]={0};
    result = recvfrom(soc, data_buf, BUF_SIZE, 0,
                              (struct sockaddr *)recv_addr, &addr_len);
    sscanf(data_buf,"%s",file_len);
    printf("%s",file_len);*/

    do /* 接收响应并保存到文件中 */
    {
       // printf("1");
        result = recvfrom(soc, data_buf, BUF_SIZE, 0,
                              (struct sockaddr *)recv_addr, &addr_len);
        if(result == 0) break;

       // printf("3");
        if (result > 0)
        {
            fwrite(data_buf, sizeof(char), result, file_ftp);

            /* 在屏幕上输出 */
            data_buf[result] = 0;
            printf("%s", data_buf);
        }
       // printf("2");
    } while(result > 0);
    //printf("fsdfsdf\n");
    fclose(file_ftp);

}
/**

*/
int uftp_send_response(SOCKET soc, char *buf, int buf_len, struct sockaddr_in* send_addr){
    //get filename
    char file_name[FILENAME_LEN];
    strcpy(file_name, buf+4);
    //文件下载
    if (strncmp(buf,"get",3)==0)  uftp_get(soc, file_name, send_addr);
    //文件上传
    else if (strncmp(buf,"put",3)==0)  uftp_put(soc, file_name, send_addr);
    else printf("the commod is not found\n");
    return 0;
}
