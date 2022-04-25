#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define PORT 5678


// Socket stream erstellen mit IPv4 und TCP
int createSocket();

void initializeSocket(struct sockaddr_in *serverAddress);

// Server Socket binden
void bindSocket(int serverSocket, struct sockaddr_in serverAddress);

// maximal 10 Clients gleichzeitig
void waitForClients(int serverSocket);

// client annehmen
int acceptClient(int serverSocket, int clientSocket, struct sockaddr *clientAddress, socklen_t clientAddressLength);

// close connection to client
void closeConnection(int clientSocket);

void clearMsg(char *msg);

#endif //SOCKET_HANDLER_H
