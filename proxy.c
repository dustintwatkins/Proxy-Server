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

void handler(int connection_fd){

    //int dest_server_fd;                     //The destination server file descriptor
    char buf[MAXLINE];
    char type[MAXLINE];
    char uri[MAXLINE];
    char version[MAXLINE];                 //Will be changing version to 1.0 always
    char hostname[MAXLINE];
    char path[MAXLINE];
    int port;

    rio_t rio_client;                      //Client rio_t
    //rio_t rio_server;                      //Server rio_t

    Rio_readinitb(&rio_client, connection_fd);
    Rio_readlineb(&rio_client, buf, MAXLINE);
    sscanf(buf,"%s %s %s", type, uri, version);

    printf("TYPE: %s\n", type);
    printf("URI: %s\n", uri);
    printf("VERSION: %s\n", version);

    /*  PARSE_URI
    *   get the hostname
    *   check if desired port is input or set to default port 80
    *   get the path from URI
    */
    parse_uri(uri, hostname, path, &port);



}


void parse_uri(char *uri, char *hostname, char * path, int *port){

    //printf("URI: %s\n", uri);

//   char* end_hostname;
    char* sub_str1 = strstr(uri, "//");
    char my_sub[MAXLINE];
    char* sub = my_sub;
    char num[5] = {'\0', '\0', '\0', '\0', '\0'};

    if(sub_str1 != NULL){
        int i = 2;                                                      //advance past the '//'
        int j = 0;
        for(; i < strlen(sub_str1); i++)
            sub[j++] = sub_str1[i];
    }
    printf("sub: %s\n", sub);                                      //sub contains everything after http://

    //Get Path
    char *sub_path = strstr(sub, "/");
    if(sub_path == NULL)
        sub_path = "/";
    path = sub_path;
    printf("PATH: %s\n", path);


    /*  Check if colon exists in sub-string
    *   if it exists, we have a designated port
    *   else set port to default port 80
    */
    char* port_substring = strstr(sub, ":");
    if(port_substring != NULL){
        int x = 1;
        int y = 0;
        while(1){
            if(port_substring[x] == '/')
                break;
            num[y++] = port_substring[x++];
        }
        *port = atoi(num);
    }
    else
        *port = 80;                                                  //Default port 80

    printf("PORT: %d\n", *port);

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
