#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h> // 추가된 헤더 파일
#include <signal.h> // signal 헤더

#define MYPORT 12345
#define BACKLOG 10

int sockfd, new_fd;


int main()
{
    
    struct sockaddr_in my_addr;    // 내 주소
    struct sockaddr_in their_addr; // 상대 주소
    int sin_size;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MYPORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind error");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen error");
        exit(1);
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
        }
        close(new_fd);
    }

    return 0;
}
