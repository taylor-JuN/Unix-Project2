#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
int ask();

//남은 시간을 출력하기 위한 구조체 선언
struct Myorder
{
    char x[BUFSIZ];
    int gettime;
    int now;
};
// 전역변수로 구조체 및 관련 변수 선언
struct Myorder myorder[10];
int i = 0;
int main()
{
    //변수 선언
    int ssock;
    int clen;
    struct sockaddr_in server_addr;
    char buf[BUFSIZ];
    char name[BUFSIZ];
    char temp[BUFSIZ];
    int index;
    //클라이언트 소켓 생성
    if ((ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket error");
        exit(1);
    }

    clen = sizeof(server_addr);
    //클라이언트 소켓 초기화
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(3317);

    //서버 소켓에 연결
    if (connect(ssock, (struct sockaddr *)&server_addr, clen) < 0)
    {
        perror("cont err");
        exit(1);
    }
    system("clear");
    printf("사용자 이름 : ");
    scanf("%s", name);
    write(ssock, name, BUFSIZ); // 이름을 입력받아 소켓에 쓰기
    while (1)
    {
        printf("1 : 메뉴 보기 | 2 : 주문 하기 | 3 : 내 주문 내역 확인 | 0 : 종료\n");
        printf("입력 : \n");
        scanf("%s", buf);
        write(ssock, buf, BUFSIZ);
        if (!strcmp(buf, "1")) // 메뉴보기
        {
            for (index = 0; index < 3; index++)
            {
                read(ssock, buf, BUFSIZ);
                printf("%s \n", buf);
            }
            printf("메뉴에 대한 설명을 보시겠습니까? [y/n] \n"); // 메뉴에 대한 상세 설명 소켓프로그래밍
            scanf("%s", buf, BUFSIZ);
            write(ssock, buf, BUFSIZ);

            read(ssock, buf, BUFSIZ);
            printf("%s", buf);
            scanf("%s", buf);

            write(ssock, buf, BUFSIZ);
            read(ssock, buf, BUFSIZ);
            printf("%s\n", buf);
        }
        if (!strcmp(buf, "2")) // 주문하기
        {
            printf("주문할 메뉴를 선택하세요! \n");
            for (index = 0; index < 3; index++)
            {
                read(ssock, buf, BUFSIZ);
                printf("%s \n", buf);
            }
            scanf("%s", buf);
            strcpy(myorder[i].x, buf); // 구조체에 이름 메뉴 이름 저장
            write(ssock, buf, BUFSIZ);
            read(ssock, buf, BUFSIZ);
            myorder[i].gettime = atoi(buf); // 구조체에 주문 시간 저장
            read(ssock, buf, BUFSIZ);
            printf("%s", buf);

            read(ssock, buf, BUFSIZ);

            myorder[i].now = atoi(buf); //구조체에 현재시간 저장
            i++;
        }
        if (!strcmp(buf, "3"))
        {
            time_t now;
            now = time(NULL);
            // 주문 건수 및 몇분 남았는지, 혹은 완료되었는지를 출력
            for (int j = 0; j < i; j++)
            {
                printf("\n");
                printf("------------------%d번째 메뉴------------------\n", j + 1);
                printf("%s메뉴는 ", myorder[j].x);
                printf("%d분이 걸립니다.\n", myorder[j].gettime);
                if ((myorder[j].gettime * 60) - (now - myorder[j].now) < 0)
                {
                    printf("완료 되었습니다.\n");
                }
                else
                {
                    printf("현재 %ld초 지났으며 ", (now - myorder[j].now));
                    printf("남은 시간은 %ld 초 입니다. \n", (myorder[j].gettime * 60) - (now - myorder[j].now));
                }
                        }
            printf("\n");
        }

        // 0을 입력받으면 프로세스 종료
        if (!strcmp(buf, "0"))
        {
            printf("종료\n");
            break;
        }
        memset(buf, 0, BUFSIZ);
    }
    close(ssock);
}
