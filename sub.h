#ifndef SUB_H
#define SUB_H

#include "keyValStore.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <ctype.h>
#include <signal.h>

#define BUFSIZE 1024 // Größe des Buffers
#define TRUE 1
#define PORT 5678
#define SEGMENT_SIZE (sizeof(*keyvalPointer))
#define MAX_CLIENTS 10

void startsocket();

#endif //SUB_H
