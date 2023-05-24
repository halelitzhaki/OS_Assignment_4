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
    struct sockaddr_in serverAddr;

    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }
    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
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