#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#define IP_ADDRESS "127.0.0.1"
#define MAX_KEYS 100;

const char key[] = "sndfgiouerj3t8et1sh8hs1h561h61ethj61h61sbsegh564r5y1hg6sehe61yeh6s4r1zh61sb6euj";

int parseOperation(char *);
int parseContentLength(char *);
int parseDividerPosition(char *header);
int decrypt(const char *key, char *value, char *buffer, int length);
int encrypt(const char *key, char *value, char *buffer, int length);
int mod(int a, int b);
int compare(const char *a, const char *b, int a_length, int b_length);
void log_msg(const char *msg);

int server_socket;

void cleanup(void)
{
    close(server_socket);
}

void sigfunc(int sig)
{
    int c;

    if (sig != SIGINT)
        return;
    else
    {
        printf("\nWollen Sie das Programm beenden (j/n) : ");
        c = getchar();
        if (c == 'j')
        {
            cleanup();
            exit(EXIT_FAILURE);
        }
        else
            return;
    }
}

int main()
{
    atexit(cleanup);
    signal(SIGINT, sigfunc);
    int client_socket, server_info_length, res;
    int operation, buffer_lenght, compare_buffer_length, content_buffer_length, new_content_length, divider_position;
    int arg_one_length, arg_two_length;
    struct sockaddr_in server_info, client_info;
    char header[100];

    // struct timeval end, start;
    // long diff;
    // int log_counter;

    // log_counter = 0;

    buffer_lenght = 0;
    compare_buffer_length = 0;
    content_buffer_length = 0;

    char *content, *buffer, *compare_buffer;

    content = (char *)malloc(1);
    buffer = (char *)malloc(1);
    compare_buffer = (char *)malloc(1);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = htonl(INADDR_ANY);
    server_info.sin_port = htons(3001);
    server_info_length = sizeof(server_info);

    bind(server_socket, (struct sockaddr *)&server_info, server_info_length);
    listen(server_socket, 1);

    srand(time(NULL));

    log_msg("Server started localhost:3001");
    /**
     * Server Algorithmus
     **/
    while (1)
    {
        memset(header, 0, 100);
        res = 0;
        client_socket = accept(server_socket, (struct sockaddr *)&client_info, &server_info_length);

        while (strstr(header, "\n\n") == NULL)
        {
            res += read(client_socket, &header[res], 1);
        }
        header[res] = 0;
        operation = parseOperation(header);
        switch (operation)
        {
        case 1:;
/* ENCRYPT */
#ifdef LOGGING
            log_msg("REQUEST TO ENCRYPT");
#endif
            // log_counter++;
            // if (log_counter % 1000 == 0)
            //     gettimeofday(&start, 0);
            new_content_length = parseContentLength(header);

            // Eventuell Speicherplatz der nötigen Werte erweitern
            if (new_content_length > buffer_lenght)
            {
                content = (char *)realloc(content, new_content_length);
                buffer = (char *)realloc(buffer, new_content_length);
                buffer_lenght = new_content_length;
                content_buffer_length = new_content_length;
            }

            // Klartext lesen
            read(client_socket, content, new_content_length);
            // Klartext verschlüsseln
            res = encrypt(key, content, buffer, new_content_length);
            // Verschlüsselten Wert zurücksenden
            write(client_socket, buffer, new_content_length);
            // Verbindung beenden
            close(client_socket);

            // if (log_counter % 1000 == 0)
            // {
            //     gettimeofday(&end, 0);
            //     printf("%i th Insertion took: %lu ms\n", log_counter, (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec);
            //     fflush(stdout);
            // }

#ifdef LOGGING
            log_msg("ENCRYPTION COMPLETED");
#endif
            break;

        case 2:;
/* DECRYPT */
#ifdef LOGGING
            log_msg("REQUEST TO DECRYPTION");
#endif

            new_content_length = parseContentLength(header);
            if (new_content_length > buffer_lenght)
            {
                content = (char *)realloc(content, new_content_length);
                buffer = (char *)realloc(buffer, new_content_length);
                buffer_lenght = new_content_length;
                content_buffer_length = new_content_length;
            }

            // read content from socket
            res = read(client_socket, content, new_content_length);

            // decrypt the Value
            res = decrypt(key, content, buffer, new_content_length);

            // Send it back
            write(client_socket, buffer, new_content_length);

            // close(client_socket);
            close(client_socket);

#ifdef LOGGING
            log_msg("DECRYPTION COMPLETED");
#endif

            break;
        case 3:;
/* COMPARE */
#ifdef LOGGING
            log_msg("REQUEST TO COMPARE");
#endif

            divider_position = parseDividerPosition(header);
            new_content_length = parseContentLength(header);

            arg_one_length = divider_position;
            arg_two_length = new_content_length - divider_position;

            if (new_content_length > content_buffer_length)
            {
                content = (char *)realloc(content, new_content_length);
                content_buffer_length = new_content_length;
            }
            if (arg_one_length > buffer_lenght)
            {
                buffer = (char *)realloc(buffer, arg_one_length);
                buffer_lenght = arg_one_length;
            }
            if (arg_two_length > compare_buffer_length)
            {
                compare_buffer = (char *)realloc(compare_buffer, arg_two_length);
                compare_buffer_length = arg_two_length;
            }

            // read content from socket
            res = read(client_socket, content, new_content_length);

            decrypt(key, content, buffer, arg_one_length);
            decrypt(key, &content[arg_one_length], compare_buffer, arg_two_length);

            res = compare(buffer, compare_buffer, divider_position, arg_two_length);

            if (res == 0)
            {
                write(client_socket, "0\n\n", 3);
            }

            if (res == -1)
            {

                write(client_socket, "-1\n", 3);
            }

            if (res == 1)
            {
                write(client_socket, "1\n\n", 3);
            }
            close(client_socket);

#ifdef LOGGING
            log_msg("COMPARISON COMPLETED");
#endif

            break;
        default:
            break;
        }
    }
}

void log_msg(const char *msg)
{
    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);

    printf("[%s] %s\n", buffer, msg);
}

int compare(const char *a, const char *b, int a_length, int b_length)
{
    if (a_length > b_length)
    {
        for (int i = 0; i < b_length; i++)
        {
            if (a[i] > b[i])
                return 1;
            if (a[i] < b[i])
                return -1;
        }
        return 1;
    }
    if (b_length > a_length)
    {
        for (int i = 0; i < a_length; i++)
        {
            if (a[i] > b[i])
                return 1;
            if (a[i] < b[i])
                return -1;
        }
        return -1;
    }
    for (int i = 0; i < a_length; i++)
    {
        if (a[i] > b[i])
            return 1;
        if (b[i] > a[i])
            return -1;
    }
    return 0;
}

int encrypt(const char *key, char *value, char *buffer, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        buffer[i] = mod((value[i] + (key[i % 80])), 127);
    }
    return length;
}

int decrypt(const char *key, char *value, char *buffer, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        buffer[i] = mod((value[i] - (key[i % 80])), 127);
    }
    return length;
}

int parseOperation(char *header)
{
    char *ptr;
    ptr = strstr(header, "Operation:");
    ptr += 10;
    return atoi(ptr);
}
int parseContentLength(char *header)
{
    char *ptr;
    ptr = strstr(header, "Content-Length:");
    ptr += 15;
    return atoi(ptr);
}
int parseDividerPosition(char *header)
{
    char *ptr;
    ptr = strstr(header, "Divider:");
    if (ptr == NULL)
    {
        perror("String parsing failed");
        exit(EXIT_FAILURE);
    }
    ptr += 8;
    return atoi(ptr);
}
int mod(int a, int b)
{
    return (a % b + b) % b;
}
