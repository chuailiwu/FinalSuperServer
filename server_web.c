#include "server.h"
/* �����ļ����Ͷ�Ӧ�� Content-Type */
struct doc_type
{
    char *suffix; /* �ļ���׺ */
    char *type;   /* Content-Type */
};

struct doc_type file_type[] =
{
    {"html",    "text/html"  },
    {"gif",     "image/gif"  },
    {"jpeg",    "image/jpeg" },
    { NULL,      NULL        }
};

char *http_res_hdr_tmpl = "HTTP/1.1 200 OK\r\nServer: Huiyong's Server <0.1>\r\n"
    "Accept-Ranges: bytes\r\nContent-Length: %d\r\nConnection: close\r\n"
    "Content-Type: %s\r\n\r\n";
/**************************************************************************
 *
 * ��������: �����ļ���׺���Ҷ�Ӧ�� Content-Type.
 *
 * ����˵��: [IN] suffix, �ļ�����׺;
 *
 * �� �� ֵ: �ɹ������ļ���Ӧ�� Content-Type, ʧ�ܷ��� NULL.
 *
 **************************************************************************/
char *http_get_type_by_suffix(const char *suffix)
{
    struct doc_type *type;

    for (type = file_type; type->suffix; type++)
    {
        if (strcmp(type->suffix, suffix) == 0)
            return type->type;
    }

    return NULL;
}

/**************************************************************************
 *
 * ��������: ����������, �õ��ļ��������׺. �����и�ʽ:
 *           [GET http://www.baidu.com:8080/index.html HTTP/1.1]
 *
 * ����˵��: [IN]  buf, �ַ���ָ������;
 *           [IN]  buflen, buf �ĳ���;
 *           [OUT] file_name, �ļ���;
 *           [OUT] suffix, �ļ�����׺;
 *
 * �� �� ֵ: void.
 *
 **************************************************************************/
void http_parse_request_cmd(char *buf, int buflen, char *file_name, char *suffix)
{
    int length = 0;
    char *begin, *end, *bias;

    /* ���� URL �Ŀ�ʼλ�� */
    begin = strchr(buf, ' ');//�����ַ���s���״γ����ַ�c��λ��
    begin += 1;

    /* ���� URL �Ľ���λ�� */
    end = strchr(begin, ' ');
    *end = 0;

    bias = strrchr(begin, '/');//��str���Ҳ࿪ʼ�����ַ�c�״γ��ֵ�λ��
    length = end - bias;

    /* �ҵ��ļ����Ŀ�ʼλ�� */
    if ((*bias == '/') || (*bias == '\\'))
    {
        bias++;
        length--;
    }

    /* �õ��ļ��� */
    if (length > 0)
    {
        memcpy(file_name, bias, length);
        file_name[length] = 0;

        begin = strchr(file_name, '.');
        if (begin)
            strcpy(suffix, begin + 1);
    }
}


/**************************************************************************
 *
 * ��������: ��ͻ��˷��� HTTP ��Ӧ.
 *
 * ����˵��: [IN]  buf, �ַ���ָ������;
 *           [IN]  buf_len, buf �ĳ���;
 *
 * �� �� ֵ: �ɹ����ط�0, ʧ�ܷ���0.
 *
 **************************************************************************/
int http_send_response(SOCKET soc, char *buf, int buf_len)
{
    int read_len, file_len, hdr_len, send_len;
    char *type;
    char read_buf[BUF_SIZE];
    char http_header[BUF_SIZE];
    char file_name[FILENAME_LEN] = "index.html", suffix[16] = "html";
    FILE *res_file;

    /* �õ��ļ����ͺ�׺ */
    http_parse_request_cmd(buf, buf_len, file_name, suffix);

    res_file = fopen(file_name, "rb+"); /* �ö����Ƹ�ʽ���ļ� */
    if (res_file == NULL)
    {
        printf("[Web] The file [%s] is not existed\n", file_name);
        return 0;
    }

    fseek(res_file, 0, SEEK_END);
    file_len = ftell(res_file);
    fseek(res_file, 0, SEEK_SET);

    type = http_get_type_by_suffix(suffix); /* �ļ���Ӧ�� Content-Type */
    if (type == NULL)
    {
        printf("[Web] There is not the related content type\n");
        return 0;
    }

    /* ���� HTTP �ײ��������� */
    hdr_len = sprintf(http_header, http_res_hdr_tmpl, file_len, type);
    send_len = send(soc, http_header, hdr_len, 0);

    if (send_len == SOCKET_ERROR)
    {
        fclose(res_file);
        printf("[Web] Fail to send, error = %d\n", WSAGetLastError());
        return 0;
    }

    do /* �����ļ�, HTTP ����Ϣ�� */
    {
        read_len = fread(read_buf, sizeof(char), BUF_SIZE, res_file);

        if (read_len > 0)
        {
            send_len = send(soc, read_buf, read_len, 0);
            file_len -= read_len;
        }
    } while ((read_len > 0) && (file_len > 0));

    fclose(res_file);

    return 1;
}
