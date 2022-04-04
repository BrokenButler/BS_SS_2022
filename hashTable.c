//
// Created by BrokenButler on 02/04/2022.
//

#include "hashTable.h"
#include <stdlib.h>
#include <string.h>

struct Ht_item {
    char *key;
    void *data;
    Ht_item *next;
};

struct Ht_table {
    Ht_item **items;
    int size;
    int count;

    unsigned long (*hash_function)(char *, unsigned long);
};

Ht_item *ht_create_item(char *key, void *data) {
    Ht_item *item = (Ht_item *) malloc(sizeof(Ht_item));
    item->key = (char *) malloc(strlen(key) + 1);
    item->data = (char *) malloc(strlen(data) + 1);
    item->next = NULL;

    strcpy(item->key, key);
    strcpy(item->data, data);

    return item;
}

Ht_table *ht_create_table(int size, unsigned long (*hash_function)(char *, unsigned long)) {
    Ht_table *table = (Ht_table *) malloc(sizeof(Ht_table));
    table->size = size;
    table->count = 0;
    table->hash_function = hash_function;
    table->items = (Ht_item **) malloc(sizeof(Ht_item *) * table->size);

    for (int i = 0; i < size; i++) {
        table->items[i] = NULL;
    }

    return table;
}

void free_item(Ht_item *item) {
    free(item->key);
    free(item->data);
    free(item->next);
    free(item);
}

void free_table(Ht_table *table) {
    for (int i = 0; i < table->size; i++) {
        Ht_item *item = table->items[i];
        while (item != NULL) {
            Ht_item *next = item->next;
            free_item(item);
            item = next;
        }
    }

    free(table->items);
    free(table);
}

int ht_insert(Ht_table *table, char *key, void *data) {
    int index = (int) (table->hash_function(key, strlen(key)) % table->size);
    Ht_item *item = table->items[index];

    while (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            item->data = data;
            return 0;
        }
        item = item->next;
    }

    item = ht_create_item(key, data);
    item->next = table->items[index];
    table->items[index] = item;
    table->count++;

    return 1;
}

int ht_remove(Ht_table *table, char *key) {
    int index = (int) (table->hash_function(key, strlen(key)) % table->size);
    Ht_item *item = table->items[index];
    Ht_item *prev = NULL;

    while (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            if (prev == NULL) {
                table->items[index] = item->next;
            } else {
                prev->next = item->next;
            }
            free_item(item);
            table->count--;
            return 1;
        }
        prev = item;
        item = item->next;
    }

    return 0;
}

void *ht_search(Ht_table *table, char *key) {
    int index = (int) (table->hash_function(key, strlen(key)) % table->size);
    Ht_item *item = table->items[index];

    while (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            return item->data;
        }
        item = item->next;
    }

    return NULL;
}
