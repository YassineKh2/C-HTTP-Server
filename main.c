#include <stdbool.h>

#include "HelperFunctions.c"
#include "SeverFunctions.c"

struct DATA
{
    char name[100];
    char value[100];
};

void GET_ROUTE(int acceptedfd, char route[])
{
    char bufftosend[5000];
    char fileURL[100];
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

void Add_Blog(struct DATA fields[])
{

    FILE *file = fopen("blog/blogs.json", "r+");
    if (!file)
    {
        perror("file not found");
        return;
    }

    // Set cursor before end

    fseek(file, 0, SEEK_END);
    fseek(file, -2, SEEK_CUR);

    const char *log_entry = ",\n{\"title\": \"Top 10 JavaScript Tips\",\"subject\": \"Web Development\",\"content\": \"Here are the top 10 JavaScript tips to improve your coding skills and productivity.\"}\n]";
    fprintf(file, "%s\n", log_entry);
    fclose(file);
}

void POST_ROUTE(int acceptedfd, char buff[])
{
    char *header200 = "HTTP/1.0 200 OK\nServer: CS241Serv v0.1\nContent-Type: text/html\n\n";
    char *header404 = "HTTP/1.0 404 Not Found\nServer: CS241Serv v0.1\nContent-Type: text/html\n\n";

    if (strlen(buff) > 1024)
        send(acceptedfd, header404, strlen(header200), 0);

    int contextlength = extract_content_length(buff);

    char reversedbuff[strlen(buff)];
    memset(reversedbuff, 0, sizeof(reversedbuff));

    char data[contextlength + 1];
    memset(data, 0, sizeof(data));

    strcpy(reversedbuff, buff);

    strrev(reversedbuff);
    strncpy(data, reversedbuff, contextlength);

    strrev(data);

    int i, nfields = 1;

    for (i = 0; i < contextlength; i++)
    {
        if (data[i] == '&')
            nfields++;
    }

    struct DATA fields[nfields];
    char field[100], value[100];
    memset(field, 0, sizeof(field));
    memset(value, 0, sizeof(value));
    int nfildsfilled = 0, counter = 0;
    bool inFiled = true;

    for (i = 0; i < contextlength + 1; i++)
    {
        // verify if we are in the same field name
        if (data[i] == '&')
        {
            strcpy(fields[nfildsfilled].name, field);
            strcpy(fields[nfildsfilled].value, value);
            nfildsfilled++;
            memset(field, 0, sizeof(field));
            memset(value, 0, sizeof(value));
            counter = 0;
            inFiled = true;
        }

        else if ((nfildsfilled == nfields - 1) && (i == contextlength))
        {
            strcpy(fields[nfildsfilled].name, field);
            strcpy(fields[nfildsfilled].value, value);
        }

        // verify if we are in the field name
        else if (data[i] == '=')
        {
            inFiled = false;
            counter = 0;
        }

        else if (inFiled)
        {
            field[counter] = data[i];
            counter++;
        }
        else if (!inFiled)
        {
            value[counter] = data[i];
            counter++;
        }
    }

    for (i = 0; i < nfildsfilled + 1; i++)
    {
        printf("field %d name : %s  value : %s \n", i, fields[i].name, fields[i].value);
    }

    Add_Blog(fields);

    // // Construct a response body with the received data
    // char responseBody[5000];
    // memset(responseBody, 0, sizeof(responseBody));
    // strcat(responseBody, "<html><body><h1>Received Data</h1><ul>");

    // for (i = 0; i < nfildsfilled + 1; i++)
    // {
    //     strcat(responseBody, "<li>");
    //     strcat(responseBody, fields[i].name);
    //     strcat(responseBody, ": ");
    //     strcat(responseBody, fields[i].value);
    //     strcat(responseBody, "</li>");
    // }

    // strcat(responseBody, "</ul></body></html>");

    // // Prepare the response header with Content-Length
    // char responseHeader[1024];
    // snprintf(responseHeader, sizeof(responseHeader),
    //          "HTTP/1.1 200 OK\r\n"
    //          "Content-Length: %ld\r\n"
    //          "Content-Type: text/html\r\n"
    //          "\r\n",
    //          strlen(responseBody));

    // // Send the response header
    // send(acceptedfd, responseHeader, strlen(responseHeader), 0);

    // // Send the response body
    // send(acceptedfd, responseBody, strlen(responseBody), 0);
}

int main()
{
    // buffer for reading data
    char *buff = malloc(INITIAL_BUFFER_SIZE);
    if (!buff)
    {
        perror("malloc");
        exit(0);
    }

    int sockfd, rbytes;
    char method[10];
    char route[100];

    // number of connction
    int fd_count = 0;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof *pfds * fd_size);

    sockfd = StartServer();
    int option = 1;

    // remove "Address already in use" error message
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

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
                    // make sure that the buff is empty
                    memset(buff, 0, strlen(buff));
                    rbytes = handle_client_data(pfds[i].fd, buff);

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

                        sscanf(buff, "%s ", method);

                        if (!strcmp(method, "GET"))
                        {
                            sscanf(buff, "GET %s HTTP/1.1", route);
                            GET_ROUTE(pfds[i].fd, route);
                        }

                        if (!strcmp(method, "POST"))
                        {
                            POST_ROUTE(pfds[i].fd, buff);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
