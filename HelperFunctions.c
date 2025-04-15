#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>


#define MAX_DATA_SIZE 8192
#define INITIAL_BUFFER_SIZE 1024

struct DATA
{
    char name[100];
    char value[100];
};

int extract_content_length(const char *request)
{
    const char *content_length_header = "Content-Length: ";
    char *content_length_pos = strstr(request, content_length_header);

    if (content_length_pos != NULL)
    {
        // Move pointer past "Content-Length: "
        content_length_pos += strlen(content_length_header);

        // Extract the number from the string
        int content_length = atoi(content_length_pos);
        return content_length;
    }
    else
    {
        // If Content-Length header not found
        printf("Content-Length header not found.\n");
        return -1; // Return -1 if Content-Length is not found
    }
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

    // Find if there is a parameter ? for put requests 
    char *queryStart = strchr(route, '?');

    // If '?' is found remove it 
    if (queryStart)
    {
        *queryStart = '\0';
    }

    if (strcmp(route, "/") == 0)
    {
        strcpy(fileURL, "index.html");
    }
    else
    {
        strcpy(fileURL, route + 1);
        FILE *file = fopen(fileURL, "r");
        if (!file)
        {
            strcpy(fileURL, "404.html");
        }
    }
}

/** strrev - reverse string, swaps src & dest each iteration.
 *  Takes valid string and reverses, original is not preserved.
 *  If str is valid, returns pointer to str, NULL otherwise.
 */
char *strrev(char *str)
{
    if (!str)
    {
        fprintf(stderr, "%s() Error: invalid string\n", __func__);
        return NULL;
    }

    char *begin = str;
    char *end = *begin ? str + strlen(str) - 1 : begin; /* ensure non-empty */
    char tmp;

    while (end > begin)
    {
        tmp = *end;
        *end-- = *begin;
        *begin++ = tmp;
    }

    return str;
}

int Add_Blog(struct DATA fields[], int contextlength, int nfildsfilled)
{

    FILE *file = fopen("blog/blogs.json", "r+");
    if (!file)
    {
        perror("file not found");
        return 0;
    }

    // Set cursor before end
    fseek(file, 0, SEEK_END);
    fseek(file, -2, SEEK_CUR);

    // We add 100 characters for the additional data like {} [] ,, and ""
    char data[contextlength + 100];
    memset(data, 0, sizeof(data));
    strcat(data, ",{");

    // Generate a random 4-digit ID
    srand(time(NULL));
    int randomID = rand() % 9000 + 1000;
    char idBuffer[12];
    snprintf(idBuffer, sizeof(idBuffer), "\"id\":%d,", randomID);
    strcat(data, idBuffer);

    int i;
    for (i = 0; i < nfildsfilled + 1; i++)
    {
        strcat(data, "\"");
        strcat(data, fields[i].name);
        strcat(data, "\":\"");
        strcat(data, fields[i].value);
        strcat(data, "\"");

        if (i != nfildsfilled)
        {
            strcat(data, ",");
        }
    }

    strcat(data, "}]");

    fprintf(file, "%s\n", data);
    fclose(file);

    return 1;
}

int Remove_Blog(int blogID)
{
    FILE *file = fopen("blog/blogs.json", "r");
    if (!file)
    {
        perror("file not found");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(fileSize + 1);
    if (!buffer)
    {
        perror("Memory allocation failed");
        fclose(file);
        return 0;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';
    fclose(file);

    char idSearch[20];
    snprintf(idSearch, sizeof(idSearch), "{\"id\":%d", blogID);
    char *start = strstr(buffer, idSearch);
    if (!start)
    {
        printf("Blog not found.\n");
        free(buffer);
        return 0;
    }

    char *end = strstr(start, "},");
    // If it didnt find },
    if (!end)
    {
        end = strstr(start, "}]");
    }
    if (end)
    {
        if (end[1] == ']')
            start--;

        end += (end[1] == ']') ? 1 : 2; 
    }

    memmove(start, end, strlen(end) + 1);
    file = fopen("blog/blogs.json", "w");
    if (!file)
    {
        perror("file not found");
        free(buffer);
        return 0;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
    free(buffer);

    return 1;
}

int Modify_Blog(int blogID, struct DATA fields[], int nfildsfilled)
{
    FILE *file = fopen("blog/blogs.json", "r");
    if (!file)
    {
        perror("file not found");
        return 0;
    }

  
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(fileSize + 1);
    if (!buffer)
    {
        perror("Memory allocation failed");
        fclose(file);
        return 0;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';
    fclose(file);

    char idSearch[20];
    snprintf(idSearch, sizeof(idSearch), "\"id\":%d", blogID);
    char *start = strstr(buffer, idSearch);
    if (!start)
    {
        printf("Blog not found.\n");
        free(buffer);
        return 0;
    }

    bool addcoma = false;
    char *end = strstr(start, "},");
    if (!end)
    {
        end = strstr(start, "}]");
    }
    if (end)
    {
        if (end[1] == ',')
            addcoma = true;

        end += (end[1] == ']') ? 1 : 2;
    }
    
    start--;
    char newBlog[1024] = "{";
    strcat(newBlog, idSearch);
    strcat(newBlog, ",");
    for (int i = 0; i < nfildsfilled; i++)
    {
        strcat(newBlog, "\"");
        strcat(newBlog, fields[i].name);
        strcat(newBlog, "\":\"");
        strcat(newBlog, fields[i].value);
        strcat(newBlog, "\"");

        if (i != nfildsfilled - 1)
        {
            strcat(newBlog, ",");
        }
    }
    strcat(newBlog, "}");
    if(addcoma)
    strcat(newBlog, ",");
    size_t newBlogLen = strlen(newBlog);
    memmove(start + newBlogLen, end, strlen(end) + 1);
    memcpy(start, newBlog, newBlogLen);

    file = fopen("blog/blogs.json", "w");
    if (!file)
    {
        perror("file not found");
        free(buffer);
        return 0;
    }

    fprintf(file, "%s", buffer);
    fclose(file);
    free(buffer);

    return 1;
}

void replace_plus_with_space(char *str)
{
    while (*str)
    {
        if (*str == '+')
        {
            *str = ' ';
        }
        str++;
    }
}
