#include "socket_handling.h"


//CREATE A SOCKET - PRINT ERROR ON STDERR AND EXIT ON FAILURE
int create_socket(int domain, int type, int protocol){

    int fd;
    fd = socket(domain, type, protocol);

    if(fd == -1){
        fprintf(stderr, "can't create socket, errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }
    
    return fd;
}

//BIND SOCKET - PRINT ERROR ON STDERR AND EXIT ON FAILURE
void bind_socket(int socket, const struct sockaddr *address, socklen_t address_len){
    int retval;
    retval = bind(socket, address, address_len);

    if(retval == -1){
        fprintf(stderr, "can't bind socket, errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }
    
}

//LISTEN FOR CONNECTIONS - PRINT ERROR ON STDERR AND EXIT ON FAILURE
void listen_for_connections(int socket, int backlog){
    int retval;
    retval = listen(socket, backlog);

    if(retval == -1){
        fprintf(stderr, "can't listen for connections, errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }
}

//ACCEPT A CONNECTION - PRINT ERROR ON STDERR AND EXIT ON FAILURE
int accept_connection(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len){
    int sock;
    sock = accept(socket, address, address_len);

    if(sock == -1){
        fprintf(stderr, "can't accept connection, errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }

    return sock;
}

//CONNECT ON SOCKET - PRINT ERROR ON STDERR AND EXIT ON FAILURE
void connect_on_socket(int s, const struct sockaddr *name, socklen_t namelen){
    int retval;

    retval = connect(s, name, namelen);

    if(retval == -1){
        fprintf(stderr, "can't connect on socket, errno:%d\n", errno);
        exit(EXIT_FAILURE);
    }
}

//READ FROM SOCKET - PRINT ERROR ON STDERR AND EXIT ON FAILURE
ssize_t read_from_socket(int fd, void* buf, size_t nbyte, char* process){
    ssize_t rbytes;
    rbytes = read(fd, buf, nbyte);
    
    if((rbytes == -1) && (errno != EINTR)){
        fprintf(stderr, "%s: can't read from socket, errno:%d\n", process, errno);
        exit(EXIT_FAILURE);
    }

    return rbytes;
}


//WRITE TO SOCKET - PRINT ERROR ON STDERR AND EXIT ON FAILURE
ssize_t write_to_socket(int fd, void* buf, size_t nbyte, char* process){
    ssize_t wbytes;
    wbytes = write(fd, buf, nbyte);

    if((wbytes == -1) && (errno != EINTR)){
        fprintf(stderr, "%s: can't write to socket, errno:%d\n", process, errno);
        exit(EXIT_FAILURE);
    }

    return wbytes;
}