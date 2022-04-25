#include "semaphore.h"

int createSemaphore() {
    int semID;
    semID = semget(IPC_PRIVATE, 1, IPC_CREAT | 0777);
    return semID;
}

void destroySemaphore(int semID) {
    // call api sem control remove semaphore by ID
    semctl(semID, 0, IPC_RMID);
}

void initializeSemaphoreOperations(struct sembuf *vOperation, struct sembuf *pOperation) {
    vOperation->sem_num = 0;
    vOperation->sem_op = 1;
    vOperation->sem_flg = 0;

    pOperation->sem_flg = 0;
    pOperation->sem_op = -1;
    pOperation->sem_num = 0;
}