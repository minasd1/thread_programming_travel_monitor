#ifndef SOCKET_HANDLING_H
#define SOCKET_HANDLING_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>


/*------------------------------FUNCTIONS TO INTERACT WITH SOCKETS--------------------------------*/
int create_socket(int domain, int type, int protocol);
void bind_socket(int socket, const struct sockaddr *address, socklen_t address_len);
void listen_for_connections(int socket, int backlog);
int accept_connection(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
void connect_on_socket(int s, const struct sockaddr *name, socklen_t namelen);
ssize_t read_from_socket(int fd, void* buf, size_t nbyte, char* process);
ssize_t write_to_socket(int fd, void* buf, size_t nbyte, char* process);


#endif