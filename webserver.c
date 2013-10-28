#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr, cli_addr; 

    char buffer[1025];
    int n, pid, clilen;
    time_t ticks; 

    /* creates unnamed socket inside the kernel and returns socket descriptor
    * AF_INET specifies IPv4 addresses
    * SOCK_STREAM specifies transport layer should be reliable
    * third argument is left 0 to default to TCP
    */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        error("ERROR opening socket");
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(buffer, '0', sizeof(buffer)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000); 

    // assigns the details specified in the structure ‘serv_addr’ to the socket created in the step above. The details include, the family/domain, the interface to listen on(in case the system has multiple interfaces to network) and the port on which the server will wait for the client requests to come.
    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding"); 

    //specifies maximum number of client connections that server will queue for this listening socket.
    listen(listenfd, 10);
    clilen = sizeof(cli_addr);

    // run in an infinite loop so that the server is always running and the delay or sleep of 1 sec ensures that this server does not eat up all of your CPU processing.
    while(1)
    {
        // the server is put to sleep and when for an incoming client request, the three way TCP handshake* is complete, the function accept () wakes up and returns the socket descriptor representing the client socket.
        connfd = accept(listenfd, (struct sockaddr*) &cli_addr, (unsigned int *) &clilen); 
        if (connfd < 0)
            error("ERROR on accept");

        // each connection gets its own forked process
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0) {
            // close file descriptor in fork
            close(listenfd);

            bzero(buffer, 1025);
            // As soon as server gets a request from client, it writes it to stdout
            n = read(connfd, buffer, 1024);
            if (n < 0)
                error("ERROR reading from socket");
            printf("%s\n", buffer);

            // end forked process
            exit(0);
        }
        else {
            close(connfd);
        }
     }
}