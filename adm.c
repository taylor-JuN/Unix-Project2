#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    //클라이언트와 같음
    int ssock;
    int clen;
    struct sockaddr_in server_addr;
    char buf[BUFSIZ];
    char name[BUFSIZ];
    char temp[BUFSIZ];
    //struct sockaddr_un ser;
    int idx;

    if ((ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    clen = sizeof(server_addr);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(3317);

    if (connect(ssock, (struct sockaddr *)&server_addr, clen) < 0)
    {
        perror("cont err");
        exit(1);
    }
    system("clear");
    printf("관리자로 로그인 : ");
    scanf("%s", name);
    write(ssock, name, BUFSIZ);
    while (1)
    {

        printf("수정을 시작하려면 start을 입력하세요 : ");
        scanf("%s", buf);

        if (!strcmp(buf, "start"))
        {
            write(ssock, buf, BUFSIZ); // 수정 할 것

            read(ssock, buf, BUFSIZ);
            printf("%s", buf);
            scanf("%s", buf);
            write(ssock, buf, BUFSIZ); //1이면 수정 2면 삭제

            read(ssock, buf, BUFSIZ);
            printf("%s", buf);
            scanf("%s", buf); // 메뉴설정
            write(ssock, buf, BUFSIZ);
            while (1)
            {
                scanf("%s", buf); // 메뉴의 상세사항을 입력받아
                if (strcmp(buf, "exit"))
                {
                    write(ssock, buf, BUFSIZ); // 소켓을 활용하여 넘겨줌
                }
                if (!strcmp(buf, "exit")) // exit를 입력받으면 끝
                {
                    write(ssock, buf, BUFSIZ);
                    break;
                }
            }
        }
        if (!strcmp(buf, "0"))
        {
            printf("종료\n");
            break;
        }
        memset(buf, 0, BUFSIZ);
    }
    close(ssock);
}