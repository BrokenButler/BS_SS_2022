//
// Created by BrokenButler on 31/03/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "hashTable.h"
#include "fnv.h"
#include "sub.h"

int main() {
    Ht_table *table = ht_create_table(10, (unsigned long (*)(char *, unsigned long)) fnv1a64);
    ht_insert(table, "Hello", "World");

    void *data = ht_search(table, "Hello");
    ht_insert(table, "Hello", "Hello World!");

    printf("%s\n", (char *) ht_search(table, "Hello"));
    printf("%s\n", (char *) data);
    return 0;
}
