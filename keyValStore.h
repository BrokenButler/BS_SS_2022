#ifndef KEYVALSTORE_H
#define KEYVALSTORE_H

#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <stdlib.h>

#define storelength 10

struct keyval {
    char key[100];
    char val[100];
};

struct keyval keyvalstore[storelength];
struct keyval *keyvalPointer;

int put(char *key, char *value);

int get(char *key, char *res);

int del(char *key);

void initialize();

#endif //KEYVALSTORE_H
