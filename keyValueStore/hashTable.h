//
// Created by BrokenButler on 02/04/2022.
//

#ifndef BS_SS_2022_HASHTABLE_H
#define BS_SS_2022_HASHTABLE_H

typedef struct Ht_item Ht_item;
typedef struct Ht_table Ht_table;

Ht_table *ht_create_table(int size, unsigned long (*hash_function)(char *, unsigned long));

Ht_item *ht_create_item(char *key, void *data);

void free_item(Ht_item *item);

void free_table(Ht_table *table);

int ht_insert(Ht_table *table, char *key, void *data);

int ht_remove(Ht_table *table, char *key);

void *ht_search(Ht_table *table, char *key);


#endif //BS_SS_2022_HASHTABLE_H
