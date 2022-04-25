//
// Created by BrokenButler on 01/04/2022.
//

#include "keyValueStore.h"

Node *head = NULL;

int put(char *key, char *value) {
    // check if key is already in the store
    char *result = (char *) malloc(100);
    if (get(key, result) == 0) {
        // if it is, delete the value
        del(key);
    }
    // create a new node
    Node *newNode = malloc(sizeof(Node));
    // set the key and value
    newNode->key = key;
    newNode->value = value;
    // set the next node to NULL
    newNode->next = NULL;
    // if the store is empty, set the head to the new node
    if (head == NULL) {
        head = newNode;
    } else {
        // otherwise, set the new node to the head
        newNode->next = head;
        head = newNode;
    }
    return 0;
}

int get(char *key, char *result) {
    // check if the store is empty
    if (head == NULL) {
        // if the store is empty, return -1
        return -1;
    }
    // set the current node to the head
    Node *current = head;
    // loop through the store
    while (current != NULL) {
        // check if the key matches the current node's key
        if (strcmp(current->key, key) == 0) {
            // if it does, return the value
            strcpy(result, current->value);
            return 0;
        }
        // otherwise, set the current node to the next node
        current = current->next;
    }
    // if the key is not found, return -1
    return -1;
}

int del(char *key) {
    // check if the store is empty
    if (head == NULL) {
        // if the store is empty, return -1
        return -1;
    }
    // set the current node to the head
    Node *current = head;
    // check if the head's key matches the key
    if (strcmp(head->key, key) == 0) {
        // if the head's key matches the key, set the head to the next node
        head = head->next;
        free(current);
        return 0;
    }
    // loop through the store
    while (current->next != NULL) {
        // check if the key matches the next node's key
        if (strcmp(current->next->key, key) == 0) {
            // save the next node
            Node *next = current->next;
            // if it does, set the current node to the next node
            current->next = current->next->next;
            // free the current node
            free(next);
            // return 0
            return 0;
        }
        // otherwise, set the current node to the next node
        current = current->next;
    }
    // if the key is not found, return -1
    return -1;
}
