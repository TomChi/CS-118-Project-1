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

/* HTTP response and header for a successful request.  */

static char* ok_response =
    "HTTP/1.1 200 OK\n"
    "Content-type: text/html\n"
    "\n";

/* HTTP response, header, and body indicating that the we didn't
   understand the request.  */

static char* bad_request_response = 
    "HTTP/1.1 400 Bad Request\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    "  <h1>Bad Request</h1>\n"
    "  <p>This server did not understand your request.</p>\n"
    " </body>\n"
    "</html>\n";

/* HTTP response, header, and body template indicating that the
   requested document was not found.  */

static char* not_found_response_template = 
    "HTTP/1.1 404 Not Found\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    "  <h1>Not Found</h1>\n"
    "  <p>The requested URL %s was not found on this server.</p>\n"
    " </body>\n"
    "</html>\n";

/* HTTP response, header, and body template indicating that the
   method was not understood.  */

static char* bad_method_response_template = 
    "HTTP/1.1 501 Method Not Implemented\n"
    "Content-type: text/html\n"
    "\n"
    "<html>\n"
    " <body>\n"
    "  <h1>Method Not Implemented</h1>\n"
    "  <p>The method %s is not implemented by this server.</p>\n"
    " </body>\n"
    "</html>\n";

void error(char *msg)
{
    perror(msg);
    exit(1);
}

static void get_resource (int requestfd, char* url) {
    printf("Retrieving resource %s\n", url);
    write(requestfd, ok_response, strlen (ok_response));
}

static void handle_request (int requestfd)
{
    char buffer[512];
    ssize_t bytes_read;

    /* Read some data from the client.  */
    bytes_read = read (requestfd, buffer, sizeof (buffer) - 1);
    if (bytes_read > 0) {
        char method[sizeof (buffer)];
        char url[sizeof (buffer)];
        char protocol[sizeof (buffer)];

        /* Some data was read successfully.  NUL-terminate the buffer so
        we can use string operations on it.  */
        buffer[bytes_read] = '\0';
        /* The first line the client sends is the HTTP request, which is
        composed of a method, the requested page, and the protocol
        version.  */
        sscanf (buffer, "%s %s %s", method, url, protocol);
        printf("Received request:\n%s", buffer);

        /* Check the protocol field.  We understand HTTP versions 1.0 and
        1.1.  */
        if (strcmp (protocol, "HTTP/1.0") && strcmp (protocol, "HTTP/1.1")) {
            /* We don't understand this protocol.  Report a bad response.  */
            write (requestfd, bad_request_response, strlen (bad_request_response));
        }
        else if (strcmp (method, "GET")) {
            /* This server only implements the GET method.  The client
            specified some other method, so report the failure.  */
            char response[1024];

            snprintf (response, sizeof (response), bad_method_response_template, method);
            write (requestfd, response, strlen (response));
        }
        else {
            /* A valid request.  Process it.  */
            get_resource (requestfd, url);

            /* Open the resource file as read only. */
            FILE * resource = fopen("test.html", "r");

            /* Read through resource line by line with a maximum of 80 characters in a line */
            if (resource != NULL) {
                char line[80];
                printf("HTTP Response Message:\n");

                /* Write to stdout, line by line. */
                while (fgets(line, sizeof(line), resource) != NULL) {
                    fputs(line, stdout);
                }

                fclose(resource);
            }
        }
    }



    else if (bytes_read == 0)
        /* The client closed the connection before sending any data.
        Nothing to do.  */
        ;
    else 
        /* The call to read failed.  */
        error("ERROR reading from request");
}

int main(int argc, char *argv[])
{
    int listenfd = 0, requestfd = 0;
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
        requestfd = accept(listenfd, (struct sockaddr*) &cli_addr, (unsigned int *) &clilen); 
        if (requestfd < 0)
            error("ERROR on accept");
        printf("Connected to %s.\n", inet_ntoa(cli_addr.sin_addr));

        // each connection gets its own forked process
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0) {
            // close file descriptor in fork
            close(listenfd);

            handle_request(requestfd);

            if (close(requestfd) < 0)
                error("ERROR closing connection");
            // end forked process
            exit(EXIT_SUCCESS);
        }
        else {
            if (close(requestfd) < 0)
                error("ERROR closing connection in parent");
            waitpid(-1, NULL, WNOHANG);
        }
    }
}
