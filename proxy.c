#include <stdio.h>
#include "csapp.h"
#include <ctype.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

//My port: 50182
//Tiny port: 50183

void handler(int connection_fd);
void parse_uri(char *uri, char *hostname, char *path, int *port);
void build_http_header(char *http_header, char *hostname, char *path, int port, rio_t *rio_client);

void handler(int connection_fd){

    int dest_server_fd;                                                         //The destination server file descriptor
    char buf[MAXLINE];
    char method[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];                                                      //Will be changing version to 1.0 always
    char hostname[MAXLINE];
    char path[MAXLINE];
    char http_header[MAXLINE];
    int port;

    rio_t rio_client;                                                           //Client rio_t
    rio_t rio_server;                                                           //Server rio_t

    Rio_readinitb(&rio_client, connection_fd);
    Rio_readlineb(&rio_client, buf, MAXLINE);
    sscanf(buf,"%s %s %s", method, uri, version);

    // printf("TYPE: %s\n", type);
    // printf("URI: %s\n", uri);
    // printf("VERSION: %s\n", version);

    if(strcasecmp(method, "GET")){
        printf("Proxy server only implements GET method\n");
        return;
    }
    /*  PARSE_URI
    *   get the hostname
    *   check if desired port is input or set to default port 80
    *   get the path from URI
    */

    memset(&path[0], 0, sizeof(path));
    memset(&hostname[0], 0, sizeof(hostname));

    //Parse the URI to get hostname, path and port
    parse_uri(uri, hostname, path, &port);

    // printf("PATH: %s\n", path);
    // printf("PORT: %d\n", port);
    // printf("HOSTNAME: %s\n", hostname);


    //Build the http header from the parsed_uri to send to server
    build_http_header(http_header, hostname, path, port, &rio_client);
    printf("%s\n", http_header);

    //Establish connection to destination server
    char port_string[100];
    sprintf(port_string, "%d", port);
    dest_server_fd = Open_clientfd(hostname, port_string);
    if(dest_server_fd < 0){
        printf("Connection to %s on port %d unsuccessful\n", hostname, port);
        return;
    }

    printf("CONNECTED!\n");

    Rio_readinitb(&rio_server, dest_server_fd);
    rio_writen(dest_server_fd, http_header, strlen(http_header));

    size_t size;
    while((size = Rio_readlineb(&rio_server, buf, MAXLINE)) != 0){
            printf("Received %zu bytes...\n", size);
            printf("Now forwarding...\n");
            rio_writen(connection_fd, buf, size);
    }
    Close(dest_server_fd);

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
    printf("PORT: %d\n", *port);

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

    char* carriage_new = "\r\n";

    char request_header[MAXLINE];
    char *request_title = "GET ";
    char *request_end = " HTTP/1.0";
    sprintf(request_header, "%s%s%s%s", request_title, path, request_end, carriage_new);

    char host_header[MAXLINE];
    char *host_title = "Host: ";
    sprintf(host_header, "%s%s%s", host_title, hostname, carriage_new);
//    printf("%s\n", host_header);

    char user_agent_header[MAXLINE];
    sprintf(user_agent_header, "%s", user_agent_hdr);
//    printf("%s\n", user_agent_header);

    char connection_header[MAXLINE];
    char *connection_title = "Connection: ";
    char *connection_field = "close";
    sprintf(connection_header, "%s%s%s", connection_title, connection_field, carriage_new);
//    printf("%s\n", connection_header);

    char proxy_connection_header[MAXLINE];
    char *proxy_title = "Proxy-Connection: ";
    char *proxy_field = "close";
    sprintf(proxy_connection_header, "%s%s%s", proxy_title, proxy_field, carriage_new);
//    printf("%s\n", proxy_connection_header);

    //NEED TO ADD ADDITIONAL REQUEST HEADERS UNCHANGED FROM ORIGINAL REQUEST

    sprintf(http_header, "%s%s%s%s%s%s",    request_header, host_header,
                                            user_agent_header, connection_header,
                                            proxy_connection_header, carriage_new);

    //printf("%s\n", http_header);
}

int main(int argc, char** argv)
{
    int listen_fd, connection_fd;
    socklen_t client_len;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    //Check Number of parameters passed in from cmd line
    if (argc != 2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    /*  Listen for incoming connections on port
    *   set listen_fd to return fd of Open_listenfd
    */
    listen_fd = Open_listenfd(argv[1]);

    while(1){
        client_len = sizeof(struct sockaddr_storage);
        connection_fd = Accept(listen_fd, (SA *)&clientaddr, &client_len);

        Getnameinfo((SA *) &clientaddr, client_len, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        handler(connection_fd);

        Close(connection_fd);
    }
    exit(0);


    printf("%s", user_agent_hdr);
    return 0;
}
