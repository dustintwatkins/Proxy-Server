#include <stdio.h>
#include "csapp.h"
#include <ctype.h>
#include "sbuf.h"
#include "cache.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREADS 4
#define SBUFSIZE 16


/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

//My port: 50182
//Tiny port: 50183

void interrupt_handler(int);
void handler(int connection_fd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *rio_client);
void *thread(void * vargp);

//TODO create CachedItem/ list
sbuf_t sbuf;

                                        //Notes for threading pg 953 txtbook
CacheList* CACHE_LIST;

void handler(int connection_fd){

    int dest_server_fd;                                                         //The destination server file descriptor
    char buf[MAXLINE];                                                          //Buffer to read from
    char method[MAXLINE];                                                       //Method, should be "GET" we don't handle anything else
    char uri[MAXLINE];                                                          //The address we are going to i.e(https://www.example.com/)
    char version[MAXLINE];                                                      //Will be changing version to 1.0 always
    char hostname[MAXLINE];                                                     //i.e. www.example.com
    char path[MAXLINE];                                                         //destinationin server i.e /home/index.html
    char http_header[MAXLINE];
    int port;

    rio_t rio_client;                                                           //Client rio_t
    rio_t rio_server;                                                           //Server rio_t

    Rio_readinitb(&rio_client, connection_fd);
    Rio_readlineb(&rio_client, buf, MAXLINE);                                   //Read first line
    sscanf(buf,"%s %s %s", method, uri, version);

    if(strcasecmp(method, "GET")){                                              //If somethinge besides "GET", disregard
        printf("Proxy server only implements GET method\n");
        return;
    }

    /*  PARSE_URI
    *   get the hostname
    *   check if desired port is input or set to default port 80
    *   get the path from URI
    */

    memset(&path[0], 0, sizeof(path));                                          //Reset the memeory of path
    memset(&hostname[0], 0, sizeof(hostname));                                  //Reset the memory of hostname

    //Parse the URI to get hostname, path and port
    parse_uri(uri, hostname, path, &port);

    //TODO check if uri is in cache


    //Build the http header from the parsed_uri to send to server
    build_http_header(http_header, hostname, path, port, &rio_client);
    //printf("%s\n", http_header);

    //Establish connection to destination server
    char port_string[100];
    sprintf(port_string, "%d", port);
    dest_server_fd = Open_clientfd(hostname, port_string);

    if(dest_server_fd < 0){
        printf("Connection to %s on port %d unsuccessful\n", hostname, port);
        return;
    }

    printf("CONNECTED!\n");

    //Send and receive info to and from destination server
    Rio_readinitb(&rio_server, dest_server_fd);
    rio_writen(dest_server_fd, http_header, sizeof(http_header));

    size_t size;
    while((size = Rio_readlineb(&rio_server, buf, MAXLINE)) != 0){
            printf("Received %zu bytes...\n", size);
            //printf("Now forwarding...\n");
            rio_writen(connection_fd, buf, size);
    }
    //Close(dest_server_fd); Used for part 1
}

void parse_uri(char *uri, char *hostname, char *path, int *port){

    char* sub_str1 = strstr(uri, "//");
    char my_sub[MAXLINE];
    memset(&my_sub[0], 0, sizeof(my_sub));
    char* sub = my_sub;
    char num[MAXLINE];
    int hostname_set = 0;

    *port = 80;                                                                 //Default port is 80

    if(sub_str1 != NULL){
        int i = 2;                                                              //advance past the '//'
        int j = 0;
        for(; i < strlen(sub_str1); i++)
            sub[j++] = sub_str1[i];
    }
    //printf("sub: %s\n", sub);                                                 //sub contains everything after http://

    /*  Check if colon exists in sub-string
    *   if it exists, we have a designated port
    *   else port is already set to default port 80
    */
    char* port_substring = strstr(sub, ":");
    if(port_substring != NULL){
        int x = 1;
        int y = 0;
        while(1){                                                               //Get port numbers
            if(port_substring[x] == '/')
                break;
            num[y++] = port_substring[x++];
        }
        *port = atoi(num);                                                      //Set port

        x = 0;
        y = 0;
        while(1){
            if(sub[y] == ':')
                break;
            hostname[x++] = sub[y++];
        }
        hostname_set = 1;
    }
    //printf("PORT: %d\n", *port);

    //Get Path
    char *sub_path = strstr(sub, "/");
    //printf("sub_path: %s\n", sub_path);
    if(sub_path != NULL){
        int a = 0;
        int b = 0;
        while(1){
            if(sub_path[b] == '\0')
                break;
            path[a++] = sub_path[b++];
        }
        if(!hostname_set){                                                      //If the hostname is not set
            a = 0;                                                              //Set it...
            b = 0;
            while(1){
                if(sub[b] == '/')
                    break;
                hostname[a++] = sub[b++];
            }
        }
    }
}

void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *rio_client){

    char buf[MAXLINE];
    char request_header[MAXLINE];
    char host_header[MAXLINE];
    char other_headers[MAXLINE];

    char *connection_header = "Connection: close\r\n";
    char *prox_header = "Proxy-Connection: close\r\n";
    char *host_header_format = "Host: %s\r\n";
    char *request_header_format = "GET %s HTTP/1.0\r\n";
    char *carriage_return = "\r\n";

    char *connection_key = "Connection";
    char *user_agent_key= "User-Agent";
    char *proxy_connection_key = "Proxy-Connection";
    char *host_key = "Host";

    int connection_len = strlen(connection_key);
    int user_len = strlen(user_agent_key);
    int proxy_len = strlen(proxy_connection_key);
    int host_len = strlen(host_key);

    sprintf(request_header, request_header_format, path);
    printf("request_header: %s\n", request_header);

    while(Rio_readlineb(rio_client, buf, MAXLINE) > 0){

            //Check for EOF first
            if(!strcmp(buf, carriage_return))
                break;

            //Check for host_key in buf
            //strncasecmp is not case sensitive
            //compares host_len chars in buf to host_key
            if(!strncasecmp(buf, host_key, host_len)){
                strcpy(host_header, buf);
                printf("HOST_HEADER: %s\n", host_header);
                continue;
            }

            //Check for any headers that are other_headers
            if( !strncasecmp(buf, connection_key, connection_len) &&
                !strncasecmp(buf, proxy_connection_key, proxy_len) &&
                !strncasecmp(buf, user_agent_key, user_len)){
                    strcat(other_headers, buf);
                }
    }

    if(strlen(host_header) == 0)                                                //If host header is not set, set it here
        sprintf(host_header, host_header_format, hostname);

    //Build the http header string
    sprintf(http_header, "%s%s%s%s%s%s%s", request_header, host_header, connection_header,
                             prox_header, user_agent_hdr, other_headers,
                             carriage_return);

    printf("HTTP_HEADERS: %s\n", http_header);
}

//Thread routine (also page 953)
//Page 972 another way of doing threads
void *thread(void *vargp){
    Pthread_detach(pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        handler(connfd);
        Close(connfd);
    }
}

void interrupt_handler(int num){
    cache_destruct(CACHE_LIST);
    free(CACHE_LIST);
    CACHE_LIST = NULL;
    exit(0);
}

//valgrind ./proxy

int main(int argc, char** argv)
{
    int listen_fd, connection_fd;
    socklen_t client_len;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t thread_id;

    //Check Number of parameters passed in from cmd line
    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    //TODO cache list/item init and set to null
    sbuf_init(&sbuf, SBUFSIZE);

    /*  Listen for incoming connections on port
    *   set listen_fd to return fd of Open_listenfd
    */
    listen_fd = Open_listenfd(argv[1]);

    CACHE_LIST = (CacheList*)malloc(sizeof(CacheList));
    cache_init(CACHE_LIST);
    signal(SIGINT, interrupt_handler);

    //Prethreading creating workiner threads
    int i;
    for(i = 0; i < NTHREADS; i++)
        Pthread_create(&thread_id, NULL, thread, NULL);

    while(1){
        client_len = sizeof(struct sockaddr_storage);
        connection_fd = Accept(listen_fd, (SA *)&clientaddr, &client_len);

        Getnameinfo((SA *) &clientaddr, client_len, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        sbuf_insert(&sbuf, connection_fd);
    }
    exit(0);

    printf("%s", user_agent_hdr);
    return 0;
}
