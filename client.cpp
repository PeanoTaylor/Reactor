#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
using std::cin;
using std::cout;
using std::endl;
int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(22222);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (ret < 0)
    {
        perror("connect");
        return -1;
    }
    
    char buf[1024];
    while (true)
    {
        cin.getline(buf, sizeof(buf));
        if (strlen(buf) == 0)
            break;
        write(sockfd, buf, strlen(buf));
        write(sockfd, "\n", 1); // 保证服务端能正确readLine
        int n = read(sockfd, buf, sizeof(buf) - 1);
        buf[n] = '\0';
        cout << ">>recv msg from server: " << buf << endl;
    }

    close(sockfd);
    return 0;
}
