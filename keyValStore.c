#include "keyValStore.h"


void initialize() {
    for (int i = 0; i <
                    storelength; i++) {                                                                             //keyvalstore wird mit Elementen aufgefüllt
        strcpy(keyvalstore[i].key, "*");
        strcpy(keyvalstore[i].val, "empty");
    }


    int shm_id_store = shmget(IPC_PRIVATE, sizeof(struct keyval) * storelength, IPC_CREAT | 0644);
    if (shm_id_store == -1) {
        perror("Das SharedMemory Segment für Store konnte nicht angelegt werden!");
        exit(1);
    }

    keyvalPointer = (struct keyval *) shmat(shm_id_store, NULL, 0);

    for (int i = 0; i <
                    storelength; i++) {                                                                                 //keyvalstore Pointer wird mit Elementen aufgefüllt
        strcpy(keyvalPointer[i].key, "*");
        strcpy(keyvalPointer[i].val, "empty");

    }
}


int put(char *key, char *value) {

    for (int i = 0;
         i < storelength; i++) {  // prüfe, ob key bereits vorhanden ist. Falls ja wird das Objekt überschrieben

        if (strcmp(keyvalPointer[i].key, key) == 0) {
            strcpy(keyvalPointer[i].key, key);
            strcpy(keyvalPointer[i].val, value);

            printf("Objekt ueberschrieben an Stelle: %i\n", i);

            return 1;
        }
    }


    for (int i = 0; i < storelength; i++) {
        if (strcmp(keyvalPointer[i].key, "*") == 0) {
            strcpy(keyvalPointer[i].key, key);
            strcpy(keyvalPointer[i].val, value);

            printf("Objekt eingefuegt an Stelle: %i\n", i);


            return 2;
        }
    }

    return 3;
}


int get(char *key, char *res) {
    for (int i = 0; i < storelength; i++) {
        if (strcmp(keyvalPointer[i].key, key) == 0) {
            strcpy(res, keyvalPointer[i].val);
            return 0;
        }
    }
    strcpy(res, "key_nonexistent");
    return -1;
}

int del(char *key) {
    for (int i = 0; i < storelength; i++) {
        if (strcmp(keyvalPointer[i].key, key) == 0) {
            strcpy(keyvalPointer[i].key, "*");
            strcpy(keyvalPointer[i].val, "empty");
            return 0;
        }
    }
    return -1;
}


