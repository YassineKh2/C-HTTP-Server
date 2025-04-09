#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_DATA_SIZE 8192
#define INITIAL_BUFFER_SIZE 1024

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

