#ifndef REACTOR_H
#define REACTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <pthread.h>

#define PORT "9034"
#define TRUE 1
#define FALSE 0
#define BUFFER 256

typedef void *(*handler_t)(void *reactor, int fd);

typedef struct node node, *pnode;
typedef struct reactor reactor, *preactor;

struct node {
    int fd;
    handler_t handler;
    pnode next;
};

struct reactor {
    pnode reactorsHead;
    pthread_t singleThread;
    struct pollfd *pollPointer;
    int isRunning;
};

void* createReactor();
void stopReactor(void*);
void startReactor(void*);
void *runReactor(void*);
void addFd(void*, int, handler_t);
void WaitFor(void*);

#endif