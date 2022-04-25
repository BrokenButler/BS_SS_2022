#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>
#include "commandHandler.h"


// start server
void startService(socklen_t clientAddressLength, struct sockaddr_in *clientAddress,
                  int clientSocket, int serverSocket, struct sembuf *vOperation, struct sembuf *pOperation);

#endif //SERVER_SERVER_H
