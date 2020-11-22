#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct hashNode {
    char *key;
    char *value;
    struct hashNode *next;
} HashNode;


typedef struct hashTable {
    int size;
    struct hashNode **table;
} HashTable;

HashTable *createTable(int size) {
    HashTable *ht = NULL;
    int i;
    
    ht = (HashTable *) malloc(sizeof(HashTable));
    if (!ht) {
        free(ht);
        return NULL;
    }

    ht->table = (HashNode **) malloc(sizeof(HashNode *) * size);
    if (!(ht->table)) {
        free(ht->table);
        free(ht);
        return NULL;
    }

    for (i = 0; i < size; ++i)
        ht->table[i] = NULL;

    ht->size = size;

    return ht;
}

int hashFunction(char *string, int size) {
    int hashPos = 0, i;
    int n = strlen(string);

    for (i = 0; i < n; ++i)
        hashPos += (int)string[i];

    return hashPos % size;
}

HashNode *newPair(char *key, char *value) {
    HashNode *pair = NULL;

    pair = malloc(sizeof(HashNode));
    if (!pair)
        return NULL;

    pair->key = malloc(strlen(key) + 1);
    if (!(pair->key)) {
        free(pair);
        return NULL;
    }

    pair->value = malloc(strlen(value) + 1);
    if (!(pair->value)) {
        free(pair->key);
        free(pair);
        return NULL;
    }

    strcpy(pair->key, key);
    strcpy(pair->value, value);
    pair->next = NULL;

    return pair;
}

int putKey(HashTable *ht, char *key, char *value) {
    HashNode *pair = NULL;
    HashNode *next = NULL;
    HashNode *last = NULL;

    int pos = hashFunction(key, ht->size);
    next = ht->table[pos];

    while (next && next->key && strcmp(key, next->key) > 0) {
        last = next;
        next = next->next;
    }

    if (next && next->key && !strcmp(key, next->key)) {
        next->value = value;
    } else {
        pair = newPair(key, value);

        if (!pair)
            return 12;

        if (next == ht->table[pos]) {
            pair->next = next;
            ht->table[pos] = pair;
        } else if (next == NULL) {
            last->next = pair;
        } else {
            pair->next = next;
            last->next = pair;
        }
    }

    return 0;
}

char *getValue(HashTable *ht, char *key) {
    int pos = hashFunction(key, ht->size);
    HashNode *pair = ht->table[pos];

    while (pair) {
        if (!strcmp(key, pair->key))
            return pair->value;

        pair = pair->next;
    }
	
    return NULL;
}

int findKey(HashTable *ht, char *key) {
    int pos = hashFunction(key, ht->size);
    HashNode *pair = ht->table[pos];
    int found = 0;

    while (pair) {
        if (!strcmp(pair->key, key))
            found = 1;

        pair = pair->next;
    }

    return found;
}

void removeKey(HashTable *ht, char *key) {
    int pos = hashFunction(key, ht->size);
    HashNode *pair = ht->table[pos];
    HashNode *temp = NULL;

    if (!pair)
        DIE(1, "No elements");

    while (pair) {
        if (!strcmp(pair->key, key)) {
            free(pair->key);
            free(pair->value);

            if (temp) {
                temp->next = pair->next;
            } else {
                ht->table[pos] = pair->next;
            }

            free(pair);

            break;
        }

        temp = pair;
        pair = pair->next;
    }
}

void destroy(HashTable *ht) {
    HashNode *temp;
    int i;

    for (i = 0; i < ht->size; ++i) {
        if ((ht->table[i])) {
            while (ht->table[i]) {
                temp = ht->table[i];
                ht->table[i] = ht->table[i]->next;
                temp->next = NULL;

                free(temp->key);
                free(temp->value);
                free(temp);
                temp = NULL;
            }
        }

        ht->table[i] = NULL;
    }

	free(ht->table);
	free(ht);
}

HashNode **getTable(HashTable *ht) {
    return ht->table;
}

char *getKey(HashNode *elem) {
    return elem->key;
}

HashNode *getNext(HashNode *elem) {
    return elem->next;
}
