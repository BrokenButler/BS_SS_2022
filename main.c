//
// Created by BrokenButler on 25/04/2022.
//

#include "server.h"

int main() {
    // socket descriptor
    int serverSocket;
    int clientSocket;

    // Server IP-Address Info
    struct sockaddr_in serverAddress;

    // Client IP-Address Info
    struct sockaddr_in clientAddress;

    socklen_t clientAddressLength;

    serverSocket = createSocket(&serverSocket);

    initializeSocket(&serverAddress);

    bindSocket(serverSocket, serverAddress);

    waitForClients(serverSocket);

    startService(clientAddressLength, &clientAddress, clientSocket, serverSocket);


    return 0;
}