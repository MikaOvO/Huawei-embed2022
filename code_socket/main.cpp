#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <iostream>
using namespace std;

#define SERVER_PORT 8889
#define SERVER_IP  "101.43.188.46"

int main(int argc, char *argv[]){

    //freopen("1.in", "r", stdin);
    //getline(cin, str);
    //char *message = (char *)str.c_str();

    int sockfd;
    struct sockaddr_in servaddr;
    int n;

    string fstr = "",str;
    while (getline(cin,str)) {
        fstr += str;
        fstr += "\n";
        if (fstr.length() <= 7) continue;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        memset(&servaddr, '\0', sizeof(struct sockaddr_in));

        servaddr.sin_family = AF_INET;
        inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
        servaddr.sin_port = htons(SERVER_PORT);

        connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        
        
        write(sockfd, (char *)fstr.c_str(), fstr.length());
        fstr = "";

        close(sockfd);
    }
    if (fstr != "") {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        
        memset(&servaddr, '\0', sizeof(struct sockaddr_in));

        servaddr.sin_family = AF_INET;
        inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
        servaddr.sin_port = htons(SERVER_PORT);

        connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        
        
        write(sockfd, (char *)fstr.c_str(), fstr.length());
        fstr = "";

        close(sockfd);    
    }
    printf("0\n0");
    return 0;
}