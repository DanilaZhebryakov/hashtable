#include <string.h>
#include <stdio.h>
#include <immintrin.h>

#include "hashtable.h"


int HTBucketCtor(HashTableBucket* bucket){
    bucket->data = (HASHT_ELEM_T *)calloc(1, sizeof(HASHT_ELEM_T));
    bucket->capacity = 1;
    bucket->size = 0;
    return (bucket->data) ? 0 : -1;
}

int HTBucketDtor(HashTableBucket* bucket){
    free(bucket->data);
    bucket->data = nullptr;
    return 0;
}

int HTBucketResize(HashTableBucket* bucket, int new_cap){
    HASHT_ELEM_T *new_data = (HASHT_ELEM_T *)realloc(bucket->data, new_cap * sizeof(HASHT_ELEM_T));
    if (!new_data) {
        return -1;
    }
    bucket->data = new_data;
    bucket->capacity = new_cap;
    return 0;
}

inline int HTBucketFind(HashTableBucket* bucket, HASHT_ELEM_T elem){
    int elsize = strlen(elem);

    if (elsize < 32) {
        __m256i elem_avx = _mm256_loadu_si256((__m256i*)(elem));
        for (int i = 0; i < bucket->size; i++){
            __m256i cmp_str = _mm256_loadu_si256((__m256i*)(bucket->data[i]));
            int cmp_res = _mm256_movemask_epi8(_mm256_cmpeq_epi8(cmp_str, elem_avx));
            if (!( ~cmp_res & (0xFFFFFFFF >> (32 - elsize - 1)) )){
                //printf("!%d %s %X %X\n", i, bucket->data[i], cmp_res, (0xFFFFFFFF >> (32 - elsize - 1)));
                return 1;
            }
        }
    }
    else {
        for (int i = 0; i < bucket->size; i++){
            if (strcmp(bucket->data[i], elem)){
                //printf("!%d %s\n", i, bucket->data[i]);
                return 1;
            }
        }
    }

    return 0;
}

int HTBucketAdd(HashTableBucket* bucket, HASHT_ELEM_T elem){
    if(HTBucketFind(bucket, elem)){
        return 1;
    }
    if (bucket->size == bucket->capacity){
        if(HTBucketResize(bucket, bucket->capacity * 2) != 0){
            return -1;
        }
    }
    bucket->data[bucket->size] = elem;
    bucket->size++;
    return 0;
}

int HTBucketRemove(HashTableBucket* bucket, HASHT_ELEM_T elem){
    for (int i = 0; i < bucket->size; i++){
        if(HASHT_ELEM_EQ(bucket->data[i], elem)){
            elem = bucket->data[bucket->size - 1];
            bucket->data[i] = elem;
            bucket->size--;
            return 0;
        }
    }
    return 1;
}

int HTBucketShrink(HashTableBucket* bucket){
    bucket->capacity = (bucket->size) ? (bucket->size) : 1;
    return HTBucketResize(bucket, bucket->capacity);
}

int hashTableCtor(HashTable* table, uint32_t ((*hash_func)(HASHT_ELEM_T))){
    table->hash_func = hash_func;
    table->buckets = (HashTableBucket*)calloc(HASH_ARR_SIZE, sizeof(HashTableBucket));
    if (!table->buckets){
        return -1;    
    }
    for(int i = 0; i < HASH_ARR_SIZE; i++){
        if(HTBucketCtor(table->buckets + i) < 0){
            return -1;
        }
    }
    return 0;
}

int hashTableDtor(HashTable* table){
    for(int i = 0; i < HASH_ARR_SIZE; i++){
        if(HTBucketDtor(table->buckets + i) < 0){
            return -1;
        }
    }
    free(table->buckets);
    table->buckets = nullptr;
    return 0;
}

int hashTableAdd(HashTable* table, HASHT_ELEM_T elem){
    uint32_t hash = table->hash_func(elem);
    return HTBucketAdd(table->buckets + (hash % HASH_ARR_SIZE), elem);
}

int hashTableFind(HashTable* table, HASHT_ELEM_T elem){
    uint32_t hash = table->hash_func(elem);

    return HTBucketFind(table->buckets + (hash % HASH_ARR_SIZE), elem);
}

int hashTableRemove(HashTable* table, HASHT_ELEM_T elem){
    uint32_t hash = table->hash_func(elem);
    return HTBucketRemove(table->buckets + (hash % HASH_ARR_SIZE), elem);
}

int hashTableShrink(HashTable* table){
    for(int i = 0; i < HASH_ARR_SIZE; i++){
        if(HTBucketShrink(table->buckets + i) < 0){
            return -1;
        }
    }
    return 0;
}

int hashTableClear(HashTable* table){
    for(int i = 0; i < HASH_ARR_SIZE; i++){
        (table->buckets + i)->size = 0;
    }
    return 0;
}
