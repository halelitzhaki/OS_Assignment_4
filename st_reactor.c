#include "reactor.h"

void* createReactor() {
    preactor _reactor = NULL;
    _reactor = malloc(sizeof(reactor));
	if(_reactor == NULL) return _reactor;

    _reactor->reactorsHead = NULL;
    _reactor->singleThread = 0;
    _reactor->pollPointer = NULL;
    _reactor->isRunning = FALSE;

    return _reactor;
}

void stopReactor(void* _reactor) {
    if (_reactor == NULL) return;

	preactor r = (preactor)_reactor;
    void *reactorThread = NULL;
	if (r->isRunning == FALSE) return;
	
	if(pthread_cancel(r->singleThread) != 0) return;
	if (pthread_join(r->singleThread, &reactorThread) != 0) return;
	if (r->pollPointer != NULL) free(r->pollPointer);

    r->pollPointer = NULL;
    r->isRunning = FALSE;
	r->singleThread = 0;
}


void* runReactor(void* _reactor) {
    if(_reactor == NULL) return NULL;
    preactor r = (preactor)_reactor;

	while (r->isRunning == TRUE) {
		pnode temp = r->reactorsHead;

        int numOfReactors = 0, i;
		while (temp != NULL) {
			numOfReactors++;
			temp = temp->next;
		}
		temp = r->reactorsHead;

		r->pollPointer = (struct pollfd*)calloc(numOfReactors, sizeof(struct pollfd));

		for(i = 0; i < numOfReactors; i++) {
			(*(r->pollPointer + i)).fd = temp->fd;
			(*(r->pollPointer + i)).events = POLLIN;
            temp = temp->next;
		}

		poll(r->pollPointer, i, -1);

		for (i = 0; i < numOfReactors; ++i) {
			if ((*(r->pollPointer + i)).revents & POLLIN) {
				pnode temp = r->reactorsHead;

				for (unsigned int j = 0; j < i; ++j) temp = temp->next;
				void *handler = temp->handler(r, (*(r->pollPointer + i)).fd);

				if (handler == NULL && (*(r->pollPointer + i)).fd != r->reactorsHead->fd) {
					pnode temp1 = r->reactorsHead, temp2 = NULL;

					while (temp1 != NULL && temp1->fd != (*(r->pollPointer + i)).fd) {
						temp2 = temp1;
						temp1 = temp1->next;
					}

					if (temp1 != NULL) {
						if (temp2 != NULL)  temp2->next = temp1->next;
						else r->reactorsHead = temp1->next;
						free(temp1);
					}
				}
			}
			else if ((((*(r->pollPointer + i)).revents & POLLNVAL) || (*(r->pollPointer + i)).revents & POLLHUP) && (*(r->pollPointer + i)).fd != r->reactorsHead->fd) {
                pnode temp1 = r->reactorsHead, temp2 = NULL;

                while (temp1 != NULL && temp2->fd != (*(r->pollPointer + i)).fd) {
                    temp2 = temp1;
                    temp1 = temp1->next;
                }
                temp2->next = temp1->next;
                free(temp1);
			}
		}

		free(r->pollPointer);
	}

	return r;
}

void startReactor(void* _reactor) {
    if (_reactor == NULL) return;

    preactor r = (preactor)_reactor;
	if (r->reactorsHead == NULL || r->isRunning == TRUE) return;
    r->isRunning = TRUE;
	
	if (pthread_create(&r->singleThread, NULL, runReactor, _reactor) != 0) r->isRunning = FALSE;
}

void addFd(void* _reactor, int _fd, handler_t _handler) {
    if (_reactor == NULL || _fd < 0 || _handler == NULL) return;

	preactor reactor = (preactor)_reactor;
	pnode newfd = (pnode)malloc(sizeof(node));

	newfd->fd = _fd;
	newfd->handler = _handler;
	newfd->next = NULL;

	if (reactor->reactorsHead == NULL) reactor->reactorsHead = newfd;
	else {
		pnode temp = reactor->reactorsHead;
		while (temp->next != NULL) temp = temp->next;
		temp->next = newfd;
	}
}


void WaitFor(void* _reactor) {
    if (_reactor == NULL) return;

	preactor r = (preactor)_reactor;
	if (r->isRunning == FALSE) return;
	
    void *reactorThread;
	if (pthread_join(r->singleThread, &reactorThread) != 0) printf("reactor's thread waiting for termination failed.\n");
}