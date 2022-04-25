#include "socketHandler.h"


int createSocket() {
    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("could not create server socket\n");
        exit(1);
    }
    return serverSocket;
}

void initializeSocket(struct sockaddr_in *serverAddress) {
    // Nur IPv4 Addressen
    serverAddress->sin_family = AF_INET;

    // Alle Netzwerkschnittstellen benutzen
    serverAddress->sin_addr.s_addr = INADDR_ANY;

    // TCP Port 5678
    serverAddress->sin_port = htons(PORT);

}

// Server Socket binden
void bindSocket(int serverSocket, struct sockaddr_in serverAddress) {
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) {
        perror("could not bind socket\n");
        exit(1);
    }
}

// maximal 10 Clients gleichzeitig
void waitForClients(int serverSocket) {
    listen(serverSocket, 10);
}

// client annehmen
int acceptClient(int serverSocket, int clientSocket, struct sockaddr *clientAddress, socklen_t clientAddressLength) {
    clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);

    if (clientSocket < 0) {
        perror("Error connection failed");
    }

    printf("Client Verbunden\n");
    return clientSocket;
}

// close connection to client
void closeConnection(int clientSocket) {
    printf("closing connection...\n");
    shutdown(clientSocket, SHUT_RDWR);
    close(clientSocket);
    printf("connection closed\n");
    exit(0);
}

void clearMsg(char *msg) {
    for (int i = 0; msg[i] != '\0'; i++) {
        msg[i] = 0;
    }
}
