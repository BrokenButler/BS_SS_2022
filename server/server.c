#include "server.h"

void startService(socklen_t clientAddressLength, struct sockaddr_in *clientAddress,
                  int clientSocket, int serverSocket, struct sembuf *vOperation, struct sembuf *pOperation) {
    while (1) {
        clientAddressLength = sizeof(clientAddress);

        clientSocket = acceptClient(serverSocket, clientSocket, clientAddress, clientAddressLength);

        // Messages
        char msg[1000];

        //command
        struct commandRequest commandHandler;
        while (strcmp(msg, "QUIT") != 0) {
            clearMsg((char *) &msg);
            clearMsg(commandHandler.command);
            // msg vom client empfangen
            read(clientSocket, msg, sizeof(msg));
            // \n\r rauswerfen
            removeTrailingLineBreak((char *) &msg);
            readCommand((char *) &msg, &commandHandler);
            executeCommand(clientSocket, commandHandler, &vOperation, &pOperation);

        }
        closeConnection(clientSocket);
    }
}