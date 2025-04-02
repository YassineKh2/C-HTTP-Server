#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <time.h>

#define PORT "3000"

int StartServer()
{

    // establishing address
    struct addrinfo hints, *res;

    // error code for getaddrinfo
    int rv;

    // Socket number
    int sockfd;

    // Getting the socket ready with info
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rv = getaddrinfo(NULL, PORT, &hints, &res);

    if (rv != 0)
    {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sockfd < 0)
    {
        perror("Couldnt create a socket \n");
        exit(2);
    }

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        close(sockfd);
        perror("Couldnt create a bind \n");
        exit(3);
    }

    freeaddrinfo(res);

    struct sockaddr_storage their_addr; // Incoming address
    socklen_t addr_size;                // Incoming address Size
    int newFd, yes = 1;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (listen(sockfd, 10) == -1)
    {
        perror("listen");
        exit(4);
    };

    newFd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    if (newFd == -1)
    {
        perror("accept");
        exit(5);
    }

    return newFd;
}

int getTimeString(char *timeBuf)
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    *timeBuf = asctime(timeinfo);
}

void GetFilePath(char route[], char *fileURL)
{

    if (!strcmp(route, "/"))
    {
        strcpy(fileURL, "index.html");
    }
    else
    {
        // ignore the / at the start
        strncpy(fileURL, route + 1, strlen(route));
        FILE *file = fopen(fileURL, "r");
        if (!file)
        {
            strcpy(fileURL, "404.html");
        }
    }
}

int main()
{
    int sockfd, acceptedfd, rbytes;
    char buff[1024];
    char bufftosend[5000];
    char method[10];
    char route[100];
    char fileURL[100];

    
    acceptedfd = StartServer();

    for (;;)
    {
        rbytes = recv(acceptedfd, buff, sizeof buff, 0);
        if (rbytes > 0)
        {
            sscanf(buff, "%s %s", method, route);
            GetFilePath(route, fileURL);
            FILE *file = fopen(fileURL, "r");
            if (!file)
            {
                perror("wrong path");
                continue;
            }

            // Get the file size
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            // Prepare the response header with Content-Length
            char resHeader[1024];
            snprintf(resHeader, sizeof(resHeader),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Length: %ld\r\n"
                     "Content-Type: text/html\r\n"
                     "\r\n",
                     file_size);

            // Send the response header
            send(acceptedfd, resHeader, strlen(resHeader), 0);

            // Send the file content
            while (fgets(bufftosend, sizeof(bufftosend), file))
            {
                send(acceptedfd, bufftosend, strlen(bufftosend), 0);
            }
            fclose(file);
        }
    }

    return 0;
}
