#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string.h>
#include "socketHandler.h"
#include "keyValueStore.h"

struct commandRequest {
    char command[255];
    char key[255];
    char value[255];
};


// interpret command
int readCommand(char *input, struct commandRequest *split);

int removeTrailingLineBreak(char *input);

// read Message from client
void readMessage(char *msg, int clientSocket);

// send message to client
void sendMessage(char *msg, int clientSocket);


int executeCommand(int clientSocket, struct commandRequest commandHandler);

int isEqual(char *str1, char *str2);

#endif //COMMAND_HANDLER_H
