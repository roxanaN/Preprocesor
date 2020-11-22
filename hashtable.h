#ifndef HASHTABLE_H_
#define HASHTABLE_H_     1

typedef struct hashNode HashNode;
typedef struct hashTable HashTable;

HashTable *createTable(int size);
HashNode **getTable(HashTable *ht);
HashNode *getNext(HashNode *elem);
char *getKey(HashNode *elem);
int hashFunction(char *string, int size);
HashNode *newPair(char *key, char *value);
int putKey(HashTable *ht, char *key, char *value);
char *getValue(HashTable *ht, char *key);
void removeKey(HashTable *ht, char *key);
int findKey(HashTable *ht, char *key);
void destroy(HashTable *ht);

#endif
