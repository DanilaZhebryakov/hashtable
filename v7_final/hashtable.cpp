#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "hashtable.h"
#include "config.h"

#ifdef DEBUG_MODE
    bool is_test_running = false;
#endif

inline uint32_t hashFuncCrc(char* str){
    uint32_t res = 5381;
    while (*str)
    {
        res = _mm_crc32_u8(res, *str);
        str++;
    }
    return res;
}

static void* slow_aligned_realloc(void* ptr, size_t size, size_t oldsize, size_t align){
    /*WHY LINUX DOES NOT HAVE ALIGNED REALLOC?????????????????????????????????????????????*/
    
    void* newptr = aligned_alloc(align, size);
    if (!newptr) {
        return nullptr;    
    }

    memcpy(newptr, ptr, oldsize);
    free(ptr);
    return newptr;
}

int HTBucketCtor(HashTableBucket* bucket){
    bucket->data = (__m256i *)aligned_alloc(sizeof(*bucket->data), 1*sizeof(*bucket->data));
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
    __m256i *new_data = (__m256i*)slow_aligned_realloc(bucket->data, new_cap * sizeof(*new_data), bucket->capacity * sizeof(*new_data) ,  sizeof(*new_data));
    if (!new_data) {
        return -1;
    }
    bucket->data = new_data;
    bucket->capacity = new_cap;
    return 0;
}

inline int HTBucketFind(HashTableBucket* bucket, HASHT_ELEM_T elem){
    __m256i elem_avx = _mm256_loadu_si256((__m256i*)(elem)); 
    __m256i half_mask = _mm256_set_epi64x(-1L ,-1L ,0 ,0);

    __m256i nochr_mask = _mm256_setzero_si256();
    nochr_mask = _mm256_cmpeq_epi8(nochr_mask, elem_avx);
    nochr_mask = _mm256_or_si256(_mm256_slli_si256(nochr_mask, 1), nochr_mask);
    nochr_mask = _mm256_or_si256(_mm256_slli_si256(nochr_mask, 2), nochr_mask);

    if (!_mm256_testz_ps((__m256)nochr_mask, (__m256)nochr_mask)) {

        nochr_mask = _mm256_or_si256(_mm256_slli_si256(nochr_mask, 4), nochr_mask);
        nochr_mask = _mm256_or_si256(_mm256_slli_si256(nochr_mask, 8), nochr_mask);
        if (!_mm256_testc_ps((__m256)half_mask, (__m256)nochr_mask)){
            nochr_mask = _mm256_or_si256(nochr_mask, half_mask);
        }
        elem_avx = _mm256_andnot_si256(nochr_mask, elem_avx); // set all past-the-end bytes to 0

        for (uint32_t i = 0; i < bucket->size; i++){
            __m256i cmp_str = _mm256_load_si256(bucket->data + i);
            __m256i cmp_res = _mm256_xor_si256 (cmp_str, elem_avx);
            //printf("%d %X %s|%s_\n", i,  cmp_res, bucket->data + i, elem);
            
            /*
            for(int j = 0; j < 32; j++){
                printf("%2X", ((char*)(bucket->data + i))[j]);
            }
            printf("\n");
            */

            if (_mm256_testz_si256(cmp_res, cmp_res)){
                //printf("!%d %s %X\n", i, bucket->data + i, cmp_res);
                #ifdef DEBUG_MODE
                    if(strcmp((char*)(bucket->data + i), elem) != 0)
                        printf("BADCMP+!%s\n", elem);
                #endif
                return i;
            }
            #ifdef DEBUG_MODE
                if(strcmp((char*)(bucket->data + i), elem) == 0)
                    printf("BADCMP-!%s\n", elem);
            #endif
        }
    }
    else {
        for (uint32_t i = 0; i < bucket->size; i++){

            if (((char*)(bucket->data + i))[sizeof(*bucket->data)-1] && (strcmp(*(char**)(bucket->data + i), elem) == 0)){
                //printf("!!%d %s\n", i, *(char*)(bucket->data + i));
                return i;
            }
        }
    }
    #ifdef DEBUG_MODE
        if(is_test_running)
            printf("NOTFOUND!%s\n", elem);
    #endif
    return -1;
}

inline int HTBucketAdd(HashTableBucket* bucket, HASHT_ELEM_T elem){
    if(HTBucketFind(bucket, elem) >= 0){
        return 1;
    }
    if (bucket->size == bucket->capacity){
        if(HTBucketResize(bucket, bucket->capacity * 2) != 0){
            return -1;
        }
    }

    unsigned int elsize = strlen(elem);
    if(elsize < sizeof(*(bucket->data))){
        strcpy((char*)(bucket->data + bucket->size), elem);
        memset((char*)(bucket->data + bucket->size) + elsize, 0, sizeof(*(bucket->data)) - elsize); //comparison is written in such way that it requires everything past-the-end to be 0
        /*
        printf(">>");
        for(int j = 0; j < 32; j++){
                printf("%2X", ((char*)(bucket->data + bucket->size))[j]);
        }
        printf("\n");
        */
    }
    else{
        *(char**)(bucket->data + bucket->size) = elem;
        ((char*) (bucket->data + bucket->size))[sizeof(*(bucket->data)) - 1] = 0xFF; //indicator byte    
    }
    bucket->size++;
    return 0;
}

int HTBucketRemove(HashTableBucket* bucket, HASHT_ELEM_T elem){
    int pos = HTBucketFind(bucket, elem);
    if(pos < 0){
        return pos;
    }
    bucket->size--;
    memcpy(bucket->data + pos, bucket->data + bucket->size, sizeof(*(bucket->data)));
    return 1;
}

int HTBucketShrink(HashTableBucket* bucket){
    bucket->capacity = (bucket->size) ? (bucket->size) : 1;
    return HTBucketResize(bucket, bucket->capacity);
}

int hashTableCtor(HashTable* table){
    table->buckets = (HashTableBucket*)calloc(HASH_ARR_SIZE, sizeof(HashTableBucket));
    if (!table->buckets){
        return -1;    
    }
    for(size_t i = 0; i < HASH_ARR_SIZE; i++){
        if(HTBucketCtor(table->buckets + i) < 0){
            return -1;
        }
    }
    return 0;
}

int hashTableDtor(HashTable* table){
    for(size_t i = 0; i < HASH_ARR_SIZE; i++){
        if(HTBucketDtor(table->buckets + i) < 0){
            return -1;
        }
    }
    free(table->buckets);
    table->buckets = nullptr;
    return 0;
}

int hashTableAdd(HashTable* table, HASHT_ELEM_T elem){
    uint32_t hash = hashFuncCrc(elem);
    return HTBucketAdd(table->buckets + (hash % HASH_ARR_SIZE), elem);
}

int hashTableFind(HashTable* table, HASHT_ELEM_T elem){
    uint32_t hash = hashFuncCrc(elem);

    int res = HTBucketFind(table->buckets + (hash % HASH_ARR_SIZE), elem);
    if (res > 0)
        return 0;
    return res;
}

int hashTableRemove(HashTable* table, HASHT_ELEM_T elem){
    uint32_t hash = hashFuncCrc(elem);
    return HTBucketRemove(table->buckets + (hash % HASH_ARR_SIZE), elem);
}

int hashTableShrink(HashTable* table){
    for(size_t i = 0; i < HASH_ARR_SIZE; i++){
        if(HTBucketShrink(table->buckets + i) < 0){
            return -1;
        }
    }
    return 0;
}

int hashTableClear(HashTable* table){
    for(size_t i = 0; i < HASH_ARR_SIZE; i++){
        (table->buckets + i)->size = 0;
    }
    return 0;
}
