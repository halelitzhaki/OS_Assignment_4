#include "reactor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>

void * _reactor = NULL;

void signalHandler();
int get_listener_socket();
void *serverHandler(void*, int);
void *clientHandler(void*, int);


int main(void)
{
    signal(SIGINT, signalHandler);
    int sockfd;     // Listening socket descriptor
    // Set up and get a listening socket
    sockfd = get_listener_socket();

    if (sockfd == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    void* _reactor = createReactor();
    addFd(_reactor, sockfd, serverHandler);
    startReactor(_reactor);
    WaitFor(_reactor);

    signalHandler();

    return 0;
}

void signalHandler() {	
	if (_reactor != NULL) {
		if (((preactor)_reactor)->isRunning) stopReactor(_reactor);
		pnode temp1 = ((preactor)_reactor)->reactorsHead, temp2 = NULL;

		while (temp1 != NULL) {
			temp2 = temp1;
			temp1 = temp1->next;
			close(temp2->fd);
			free(temp2);
		}
		free(_reactor);
	}
    exit(0);
}

int get_listener_socket()
{
    int fd, opt=1;
    struct sockaddr_in serverAddr;

    // Get us a socket and bind it
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = PORT;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
        
    // Lose the pesky "address already in use" error message
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

        if (bind(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
            close(fd);
            printf("bind failed");
            return -1;
        }


    // Listen
    if (listen(fd, 10) == -1) {
        return -1;
    }

    return fd;
}

void *serverHandler(void* _reactor, int sock_fd) {
    struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	int client_fd = accept(sock_fd, (struct sockaddr *)&clientAddr, &clientAddrLen);

    preactor r = (preactor)_reactor;
	addFd(r, client_fd, clientHandler);

	return _reactor;
}

void *clientHandler(void* _reactor, int sock_fd) {
    char buffer[BUFFER], copy[BUFFER];

	int bytes = recv(sock_fd, buffer, BUFFER, 0);
    buffer[BUFFER-1] = '\0';
    if (bytes <= 0) {
        return NULL;
    }

    strcpy(copy, buffer);
    pnode temp = ((preactor)_reactor)->reactorsHead->next;

    printf("%s", buffer);

    while (temp != NULL) {
        if (temp->fd != sock_fd) send(temp->fd, copy, BUFFER, 0);
        temp = temp->next;
    }

	return _reactor;
}