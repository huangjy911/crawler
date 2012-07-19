#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include "Spliter.h"
#include "httplib.h"
#include "url.h"

FILE* fp;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_sock = PTHREAD_MUTEX_INITIALIZER;
static int total;

#define URLMAXLEN 255

const char * USEAGE = 
"Useage : %s -f filename [OPTION]\n"
"OPTION:\n"
"  -n  integer        设置线程数量\n";
;

void* fetch(void* arg)
{
    char url[URLMAXLEN];
    int flag;

    Spliter spliter("");
    Http_handle http_handle("", 0, "");

    while (true)
    {
        pthread_mutex_lock(&mutex_file);
        if ((fgets(url, URLMAXLEN, fp)) == NULL)
        {
            total--;
            pthread_mutex_unlock(&mutex_file);
            return NULL;
        }
        pthread_mutex_unlock(&mutex_file);

        rstrip(url);
        if (strlen(url) == 0) continue;
        // TODO: url fix
        
        spliter.reset(url);
        spliter.exec();

        // TODO: tolower
        if (spliter.get_proto() && strcmp(spliter.get_proto(), "http") != 0)
        {
            pthread_mutex_lock(&mutex);
            spliter.print();
            printf("Sorry, I can only deal http.\n");
            printf("<<----------------------\n");
            pthread_mutex_unlock(&mutex);
            continue;
        }

        http_handle.reset(spliter.get_domin(), spliter.get_port(), spliter.get_path());
        flag = http_handle.get_socket();
        if (flag == -1)
        {
            pthread_mutex_lock(&mutex);
            spliter.print();
            printf("get_socket error\n");
            printf("<<----------------------\n");
            pthread_mutex_unlock(&mutex);
            continue;
        }

        pthread_mutex_lock(&mutex_sock);
        flag = http_handle.socket_connect();
        pthread_mutex_unlock(&mutex_sock);
        if (flag == -1)
        {
            pthread_mutex_lock(&mutex);
            spliter.print();
            printf("connect error\n");
            printf("<<----------------------\n");
            pthread_mutex_unlock(&mutex);
            continue;
        }

        flag = http_handle.request();
        if (flag == -1)
        {
            pthread_mutex_lock(&mutex);
            spliter.print();
            printf("io error\n");
            printf("<<----------------------\n");
            pthread_mutex_unlock(&mutex);
            continue;
        }
    
        pthread_mutex_lock(&mutex);
        spliter.print();
        http_handle.print_abstract();
        printf("<<----------------------\n");
        pthread_mutex_unlock(&mutex);
        continue;
    }
    return NULL;
}

// int main(int argc, char *argv[])
// {
//     if (argc != 2)
//     {
//         perror("usage: url_fetch filename\n");
//         exit(1);
//     }
//     if ((fp = fopen(argv[1], "r")) == NULL)
//     {
//         perror("File open error!\n");
//         exit(1);
//     }
//     total = 5;
//     for (int i = 0; i < total; ++i)
//     {
//         pthread_t id;
//         pthread_create(&id, NULL, fetch, NULL);
//     }
//     while (true)
//     {
//         sleep(1);
//         if (total == 0)
//             break;
//     };
//     fclose(fp);
//     return 0;
// }

void show_usage(const char *proc_name)
{
    printf(USEAGE, proc_name);
    return;
}

int main(int argc, char *argv[])
{
    int opt;
    int pthread_count = 5;
    char* file;

    while ((opt = getopt(argc, argv, "f:n:")) != -1) {
        switch (opt) {
        case 'n':
            pthread_count = atoi(optarg);
            break;
        case 'f':
            file = strdup(optarg);
            break;
        }
    }

    if (NULL == file) {
        show_usage(argv[0]);
        return -1;
    }
    
    return 0;
}
