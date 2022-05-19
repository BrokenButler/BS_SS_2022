#ifndef SUB_H
#define SUB_H

#include "keyValStore.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <netinet/in.h>

#define BUFSIZE 1024 // Größe des Buffers
#define ENDLOSSCHLEIFE 1
#define PORT 5678
#define SEGMENT_SIZE (sizeof(*keyvalPointer))


void startsocket();

#endif //SUB_H
