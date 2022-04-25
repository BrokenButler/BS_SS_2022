#include "commandHandler.h"


int readCommand(char *input, struct commandRequest *split) {
    char *token = strtok(input, " ");
    if (token == NULL) {
        return -1;
    }
    strcpy(split->command, token);

    token = strtok(NULL, " ");
    if (token == NULL) {
        return -1;
    }
    strcpy(split->key, token);

    token = strtok(NULL, " ");
    if (token != NULL) {
        strcpy(split->value, token);
    }
    return 0;
}


int removeTrailingLineBreak(char *input) {
    input[strcspn(input, "\n")] = 0;
    input[strcspn(input, "\r")] = 0;
    return 0;
}


int executeCommand(int clientSocket, struct commandRequest commandHandler, struct sembuf *vOperation,
                   struct sembuf *pOperation) {


    if (strcmp("PUT", commandHandler.command) == 0) {
        char input[1000] = "value stored \n";
        put(commandHandler.key, commandHandler.value);
        clearMsg(commandHandler.command);
        write(clientSocket, input, sizeof(input));
    } else if (strcmp("GET", commandHandler.command) == 0) {
        char input[100] = "";
        get(commandHandler.key, commandHandler.value);
        clearMsg(commandHandler.command);
        strcat(input, commandHandler.key);
        strcat(input, ":");
        strcat(input, commandHandler.value);
        strcat(input, "\n");
        write(clientSocket, input, sizeof(input));
    } else if (strcmp("DEL", commandHandler.command) == 0) {
        char input[1000] = "Key Value deleted\n";
        del(commandHandler.key);
        clearMsg(commandHandler.command);
        write(clientSocket, input, sizeof(input));
    } else if (strcmp("END", commandHandler.command) == 0) {
//        end();
    } else if (strcmp("SUB", commandHandler.command) == 0) {
//        sub(commandHandler.key);
    }
    return 0;
}


