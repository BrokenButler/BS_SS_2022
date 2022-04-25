#ifndef SERVER_SEMAPHORE_H
#define SERVER_SEMAPHORE_H

#include <sys/sem.h>
#include <semaphore.h>

int createSemaphore();

void destroySemaphore(int semID);

void initializeSemaphoreOperations(struct sembuf *vOperation, struct sembuf *pOperation);

#endif //SERVER_SEMAPHORE_H
