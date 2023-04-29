#include <stdlib.h>
#include <stdint.h>

#define HASHT_ELEM_T char*
#define HASHT_ELEM_EQ(a,b) (strcmp(a,b) == 0)

const size_t HASH_ARR_SIZE = 613;

struct HashTableBucket{
    HASHT_ELEM_T *data;
    uint32_t size;      // I think that 10^9 elements per line is surely out of memory and is not hashtabl-y either
    uint32_t capacity;  // but having structure take 128 bits is nice
};

struct HashTable{
    uint32_t ((*hash_func)(HASHT_ELEM_T));
    HashTableBucket* buckets;
};

int hashTableCtor(HashTable* table, uint32_t ((*hash_func)(HASHT_ELEM_T)));

int hashTableDtor(HashTable* table);

int hashTableAdd(HashTable* table, HASHT_ELEM_T elem);

int hashTableFind(HashTable* table, HASHT_ELEM_T elem);

int hashTableRemove(HashTable* table, HASHT_ELEM_T elem);

int hashTableShrink(HashTable* table);

int hashTableClear(HashTable* table);