//
// Created by kalesh on 4/15/19.
//

#include <stdio.h>
#include "connection.h"
#include "server-threads.h"

// Global constants
#define DEFAULT_PORT 2002
#define DEFAULT_IP_ADDR "127.0.0.1"

int main(int argc, char* argv[]) {
    int list_sock;                  // Listening socket
    short port;                     // Port number (default 2002)
    char* ip_addr;                  // Ip Address (default "127.0.0.1")
    struct sockaddr_in serv_addr;   // Socket address structure

    // TODO: Parse the command line and set the default
    // ports and IP address if no arguments were supplied.
    port = DEFAULT_PORT;
    ip_addr = DEFAULT_IP_ADDR;

    // Create listening socket
    list_sock = Socket(AF_INET, SOCK_STREAM, 0);

    // Initialize the socket address struct
    init_sockaddr(&serv_addr, ip_addr, port);

    // Bind socket address to listening socket
    Bind(list_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    // Start listening
    Listen(list_sock);

    // Initialize the client connections
    init_client_conns();

    // Infinite while loop to respond to client requests
    while (1) {
        // Block until there is a connection request, then accept it
        int sockfd = Accept(list_sock, NULL, NULL);

        // Add the client connection
        add_client_conn(sockfd);
    }
}