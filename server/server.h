#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include "keyValueStore.h"


#define BUFSIZE 1024
#define PORT 5678
#define SEGMENT_SIZE (sizeof(*keyvalPointer))

struct kvp {
    char key[30];
    char value[80];
};


// start server
void startService();

#endif //SERVER_H
