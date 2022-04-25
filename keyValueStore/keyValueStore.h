//
// Created by BrokenButler on 01/04/2022.
//

#ifndef BS_SS_2022_KEYVALUESTORE_H
#define BS_SS_2022_KEYVALUESTORE_H

#include <stdlib.h>
#include <string.h>
//#include "semaphore.h"

typedef struct Node {
    char *key;
    char *value;
    struct Node *next;
} Node;

int put(char *key, char *value);

int get(char *key, char *result);

int del(char *key);

#endif //BS_SS_2022_KEYVALUESTORE_H
