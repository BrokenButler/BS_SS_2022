#ifndef KEYVALUESTORE_H
#define KEYVALUESTORE_H

#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>

#define storelength 100

struct keyval {
    char key[30];
    char val[80];
};

struct keyval keyvalstore[storelength];
struct keyval *keyvalPointer;

void init_store();

int put(char *key, char *value);

int get(char *key, char *result);

int del(char *key);

#endif //KEYVALUESTORE_H
