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

    if (listen(sockfd, 10) == -1)
    {
        perror("listen");
        exit(4);
    };

    return sockfd;
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

void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;

    printf("new sock yay ! %d \n", newfd);
}

void HandleNewConnection(int sockfd, struct pollfd *pfds[], int *fd_count, int *fd_size)
{

    struct sockaddr_storage remoteaddr;
    socklen_t addrlen = sizeof(remoteaddr);
    int newfd;

    newfd = accept(sockfd, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfd == -1)
    {
        perror("accept");
    }
    else
    {
        add_to_pfds(pfds, newfd, fd_count, fd_size);
    }
}

void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1];

    (*fd_count)--;
}

void GET_ROUTE(int acceptedfd, char route[], char fileURL[])
{
    char bufftosend[5000];
    GetFilePath(route, fileURL);
    FILE *file = fopen(fileURL, "r");
    if (!file)
    {
        perror("wrong path");
        return;
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
    // TO FIX : make sure it takes the correct length of the file its sending
    while (fgets(bufftosend, sizeof(bufftosend), file))
    {
        send(acceptedfd, bufftosend, strlen(bufftosend), 0);
    }
    fclose(file);
}

int main()
{

    int sockfd, rbytes;
    char buff[1024];
    char method[10];
    char route[100];
    char fileURL[100];

    // number of connction
    int fd_count = 0;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    sockfd = StartServer();
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Add the listener to set
    pfds[0].fd = sockfd;
    // Report ready to read on incoming connection
    pfds[0].events = POLLIN;
    // for the socket
    fd_count = 1;

    for (;;)
    {

        int poll_count = poll(pfds, fd_count, -1);
        // check if still have an open connection
        if (poll_count == -1)
        {
            perror("poll");
            exit(1);
        }

        for (int i = 0; i < fd_count; i++)
        {
            // Check if someone's ready to read
            if (pfds[i].revents & (POLLIN | POLLHUP))
            {
                // Handle new connection
                if (pfds[i].fd == sockfd)
                {
                    HandleNewConnection(sockfd, &pfds, &fd_count, &fd_size);
                }
                else
                {

                    rbytes = recv(pfds[i].fd, buff, sizeof(buff), 0);

                    if (rbytes <= 0)
                    {
                        // Client disconnected or error occurred
                        if (rbytes == 0)
                        {
                            printf("Socket %d disconnected\n", pfds[i].fd);
                        }
                        else
                        {
                            perror("recv");
                        }

                        // Close the socket and remove it from the pollfd array
                        close(pfds[i].fd);
                        del_from_pfds(pfds, i, &fd_count);
                    }
                    else
                    {

                        printf("sending to %d", pfds[i].fd);
                        sscanf(buff, "%s %s", method, route);

                        if (!strcmp(method, "GET"))
                            GET_ROUTE(pfds[i].fd, route, fileURL);
                    }
                }
            }
        }
    }

    return 0;
}
