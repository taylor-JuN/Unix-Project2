#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

void *do_thread(void *arg);
void getMenu();
int getMenuDetail(char *str, char *nick, int sockfd);
int order(char *str, int sockfd);
void modMenu(char *str, int sockfd);
void delMenu(char *str, int sockfd);

//전역변수 선언
int count;
struct dirent **menulist;

int main()
{
    //소켓프로그래밍에 필요한 소켓 선언
    int ssock, csock;
    unsigned int clen;
    struct sockaddr_in client_addr, server_addr;

    //스레드 구조체 선언
    pthread_t tid;

    //서버 소켓 선언
    if ((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error : ");
        exit(1);
    }

    clen = sizeof(client_addr);
    //서버 소켓 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(3317);

    //바인드
    if (bind(ssock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        exit(1);
    }
    // 클라이언트 대기
    if (listen(ssock, 8) < 0)
    {
        perror("listen error");
        exit(1);
    }
    //클라이언트가 연결되면 실행
    while (1)
    {
        //클라이언트마다 새로운 스레드를 생성한 후 do_thread실행
        csock = accept(ssock, (struct sockaddr *)&client_addr, &clen);
        printf("소켓 번호 는 %d입니다 \n", csock);
        if (pthread_create(&tid, NULL, do_thread, (void *)&csock) < 0)
        {
            perror("thread err");
            exit(1);
        }
    }
}

void *do_thread(void *arg)
{
    //필요한 변수 선언
    int sockfd;
    char buf[BUFSIZ];
    char nick[BUFSIZ];
    int index;
    char *str;
    FILE *fp;
    int c;
    int ordertime;
    char path[1024] = "/Users/taylor/prog/code/unix/project2/menu/";
    char recipe_buf[BUFSIZ];
    char ot[BUFSIZ];

    //인자로 넘겨받은 arg를 파일 디스크립터로 캐스팅
    sockfd = *((int *)arg);
    //닉네임을 nick에 저장 admin과 분리하기 위함
    read(sockfd, nick, BUFSIZ);

    // 닉네임이 admin이라면
    if (!strcmp(nick, "admin"))
    {
        while (1)
        {
            //start를 입력 받는다면
            read(sockfd, buf, BUFSIZ);
            if (!strcmp(buf, "start"))
            {
                //수정 및 삭제를 선택
                strcpy(buf, "mod: 수정, del: 삭제");
                write(sockfd, buf, BUFSIZ);
                read(sockfd, buf, BUFSIZ);
                if (!strcmp(buf, "mod"))
                {
                    //메뉴 입력
                    strcpy(buf, "수정할 메뉴를 입력하세요\n");
                    write(sockfd, buf, BUFSIZ);
                    read(sockfd, buf, BUFSIZ);
                    //메뉴 수정 함수 실행
                    modMenu(buf, sockfd);
                }
                else if (!strcmp(buf, "del"))
                {
                    //메뉴 입력
                    strcpy(buf, "삭제할 메뉴를 입력하세요\n");
                    write(sockfd, buf, BUFSIZ);
                    read(sockfd, buf, BUFSIZ);
                    //메뉴 삭제 함수 실행
                    delMenu(buf, sockfd);
                }
            }
        }
    }
    else // 클라이언트라면
    {
        while (1)
        {
            //1,2,3,0 선택
            read(sockfd, buf, BUFSIZ);
            if (!strcmp(buf, "1"))
            {
                printf("%d번 고객이 메뉴를 보고 있습니다 \n", sockfd);
                //메뉴보여주는 함수 실행
                getMenu();
                //메뉴의 갯수 만큼 실행
                for (index = 0; index < count; index++)
                {
                    //현재 디랙토리는 출력하지 않음
                    if (menulist[index]->d_name[0] == '.')
                    {
                        continue;
                    }
                    else
                    {
                        //이 외 이름을 str에 저장
                        str = menulist[index]->d_name;
                    }
                    //소켓파일디스크립터에 저장
                    strcpy(buf, str);
                    write(sockfd, buf, BUFSIZ);
                }
                //메뉴 상세를 보고 싶다면
                read(sockfd, buf, BUFSIZ);

                if (!strcmp(buf, "y"))
                {
                    strcpy(buf, "메뉴 이름을 말하세요 \n");
                    write(sockfd, buf, BUFSIZ);

                    read(sockfd, buf, BUFSIZ);
                    //메뉴디테일 함수 실행
                    getMenuDetail(buf, nick, sockfd);
                }
            }
            //2번 주문 실행
            if (!strcmp(buf, "2"))
            {
                //메뉴를 갖고와서 보여줌
                getMenu();
                for (index = 0; index < count; index++)
                {
                    if (menulist[index]->d_name[0] == '.' || menulist[index]->d_name[0] == 'i')
                    {
                        continue;
                    }
                    else
                    {
                        str = menulist[index]->d_name;
                    }
                    strcpy(buf, str);
                    write(sockfd, buf, BUFSIZ);
                }
                read(sockfd, buf, BUFSIZ); // 위와 같음

                ordertime = order(buf, sockfd); // 현재 주문시간을 리턴받아 변수에 저장
                sprintf(ot, "%d", ordertime);   // 타입캐스팅

                write(sockfd, ot, BUFSIZ); //소켓파일디스크립터로 타입캐스팅된 현재시간 write
            }

            if (!strcmp(buf, "3")) // 현재 시간으로 돌아가기 때문에 서버측에서는 아무것도 하지않음
            {
            }

            if (!strcmp(buf, "0")) // 영업 종료
            {
                printf("영업 종료\n");
                break;
            }
        }
    }
    close(sockfd); // 소켓파일디스크립터 닫음
    pthread_exit(NULL);
}

void getMenu()
{
    // 디렉토리 오픈
    if (opendir("/Users/taylor/prog/code/unix/project2/menu") == NULL)
    {
        printf("메뉴가 없슴!");
        exit(1);
    }
    // 디렉토리를 스캔하여 변수에 갯수 저장
    if ((count = scandir("/Users/taylor/prog/code/unix/project2/menu", &menulist, NULL, alphasort)) == -1)
    {
        fprintf(stderr, "scan error ! \n");
        exit(1);
    }
}

int getMenuDetail(char *str, char *nick, int sockfd)
{
    //변수 선언
    int fd;
    int c;
    char buf[BUFSIZ];
    char path[BUFSIZ] = "/Users/taylor/prog/code/unix/project2/menu/";
    //인자로 넘어온 str과 path를 붙혀 메뉴 고름
    strcat(path, str);

    //파일디스크립터로 파일 오픈, 에러면 메뉴가 없는 것
    if ((fd = open(path, O_RDONLY)) == -1)
    {
        strcpy(buf, "메뉴가 없습니다\n");
        write(sockfd, buf, BUFSIZ);
        return 0;
    }
    //소켓파일디스크립터에 buf저장 -> 파일내용 write
    c = read(fd, buf, BUFSIZ);
    write(sockfd, buf, BUFSIZ);

    return 0;
}

int order(char *str, int sockfd)
{
    //변수 선언
    time_t now;
    now = time(NULL);
    int fp;
    char buf[BUFSIZ];
    char expl[BUFSIZ];
    char path[BUFSIZ] = "/Users/taylor/prog/code/unix/project2/menu/";
    //메뉴 선정
    strcat(path, str);

    if ((fp = open(path, O_RDONLY)) == -1)
    {
        strcpy(buf, "메뉴가 없습니다\n");
        write(sockfd, buf, BUFSIZ);
        return 0;
    }
    else
    {
        printf("%s 메뉴 주문이 들어왔습니다. 몇 분 걸립니까? 품절시 'stop'을 입력하세요\n", str);
        scanf("%s", expl);           // 서버입장에서 몇 분 걸리는지 입력
        write(sockfd, expl, BUFSIZ); // 파일디스크립터에 입력
        if (!strcmp(expl, "stop"))
        {
            strcpy(expl, "죄송합니다. 해당 메뉴는 품절 되었습니다.\n");
        }
        else
        {
            strcpy(str, "분 후에 찾으러 오세요\n");
            strcat(expl, str);
        }

        write(sockfd, expl, BUFSIZ);
    }
    return now;
}

void modMenu(char *str, int sockfd)
{
    int fd;
    int c;
    char buf[BUFSIZ];
    char x[BUFSIZ];
    char path[BUFSIZ] = "/Users/taylor/prog/code/unix/project2/menu/";
    strcat(path, str);

    // 읽기 권한이기에 없으면 파일을 생성
    if (((fd = open(strcat(path, buf), O_RDWR | O_CREAT | O_TRUNC, 0644)) < 0))
    {
        perror("cannot open file");
        exit(1);
    }
    // exit를 받기 전까지 입력을 계속 받아 새로운 파일에 입력받은 값을 파일출력함, "exit"입력시 파일에 출력되지 않고 while문 탈출
    while (1)
    {
        read(sockfd, x, BUFSIZ);
        if ((c = write(fd, x, sizeof(x))) < 0)
        {
            perror("write err");
        }
        else if (!strcmp(x, "exit"))
        {
            printf("수정 완료!\n");
            break;
        }
    }
    close(fd);
}

// 메뉴 삭제
void delMenu(char *str, int sockfd)
{
    char path[BUFSIZ] = "rm -f /Users/taylor/prog/code/unix/project2/menu/";
    strcat(path, str);
    system(path);
}
