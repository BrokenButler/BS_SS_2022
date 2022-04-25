#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>
#include "commandHandler.h"


// start server
void startService(socklen_t clientAddressLength, struct sockaddr_in *clientAddress,
                  int clientSocket, int serverSocket);

#endif //SERVER_H
