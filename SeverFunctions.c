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

int handle_client_data(int fd, char *buff)
{

    size_t total_bytes_read = 0;
    ssize_t rbytes;
    size_t buffer_size = INITIAL_BUFFER_SIZE;

    while (total_bytes_read < MAX_DATA_SIZE)
    {
        if (total_bytes_read + 1 >= buffer_size)
        {
            buffer_size *= 2;
            char *new_buff = realloc(buff, buffer_size);
            if (!new_buff)
            {
                perror("realloc");
                free(buff);
                return -2;
            }
            buff = new_buff;
        }

        rbytes = recv(fd, buff + total_bytes_read, buffer_size - total_bytes_read - 1, 0);
        if (rbytes <= 0)
        {
            if (rbytes == 0)
            {
                printf("Socket %d disconnected\n", fd);
                return 0;
            }
            else
            {
                perror("recv");
                return -1;
            }
            break;
        }

        total_bytes_read += rbytes;
        buff[total_bytes_read] = '\0';
        return 1;
    }

    if (total_bytes_read >= MAX_DATA_SIZE)
    {
        printf("Maximum data size exceeded, closing connection.\n");
        return -1;
    }

    free(buff);
}
