#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h> // 추가된 헤더 파일
#include <signal.h>    // signal 헤더

#define MYPORT 12345
#define BACKLOG 10

int sockfd, new_fd;

void signIntHandler(int sig_num)
{
    printf("/n Ctrl+C 입력으로 서버를 종료합니다.\n");
    close(sockfd);
    exit(EXIT_SUCCESS);
}

int main()
{

    struct sockaddr_in my_addr;    // 내 주소
    struct sockaddr_in their_addr; // 상대 주소
    int sin_size;

    // ctrl + c를 눌러스 프로세스를 종료 -> 그러나 프로세스는 종료되지만 종료된 프로세스에서 바인딩한  "port"는 한동안 release 되지 않고
    // OS가 들고 있는다. 그래서서 똑같은 port를 쓰려면 문제가 발생
    // 따라서 signal를 사용하여, ctrl + c를 눌렀을때, 프로세스가 종료되면서 관련된 socket 자료구조들이 release하게 한다.
    signal(SIGINT, signIntHandler);
    // signal(SIGTERM, signIntHandler) 와 차이가 있다. 3장 - 26page 참고

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYPORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror("accept error");
            continue;
        }
        printf("server : got connection form %s\n", inet_ntoa(their_addr.sin_addr));

        char request[1024];
        int bytes_received = recv(new_fd, request, sizeof(request), 0);
        if (bytes_received > 0)
        {
            request[bytes_received] = '\0';
            printf("HTTP Request:\n%s\n", request);

            // HTTP 요청 파싱 - 요청된 파일명 추출 (간단히 GET 메서드로부터 추출)
            char *requested_file = strtok(request, " ");
            requested_file = strtok(NULL, " "); // 요청된 파일명

            // 요청된 파일이 "/index.html"인지 확인하고 해당 파일 로드
            if (strcmp(requested_file, "/index.html") == 0)
            {
                FILE *file_ptr;
                char file_buffer[1024];
                memset(file_buffer, 0, sizeof(file_buffer));

                if ((file_ptr = fopen("index.html", "r")) == NULL)
                {
                    printf("File not found\n");
                    // 파일을 찾을 수 없는 경우 404 응답 전송
                    char http_404_response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
                    send(new_fd, http_404_response, strlen(http_404_response), 0);
                }
                else
                {
                    fread(file_buffer, sizeof(char), sizeof(file_buffer), file_ptr);
                    fclose(file_ptr);

                    // HTTP 응답 생성 - 요청된 파일을 포함한 HTTP 응답 생성
                    char http_response[2048];
                    sprintf(http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n%s", file_buffer);

                    // 클라이언트에게 응답 보내기
                    send(new_fd, http_response, strlen(http_response), 0);
                }
            }
            else
            {
                // 다른 파일을 요청한 경우 404 응답 전송
                char http_404_response[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1>404 Not Found</h1>";
                send(new_fd, http_404_response, strlen(http_404_response), 0);
            }
        }
        close(new_fd);
    }

    return 0;
}
