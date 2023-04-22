#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "lib/String.h"
#include "lib/file_read.h"
#include "hashtable.h"


uint32_t hashFuncFirst(char* str){
    return *((unsigned char*)str);
}
uint32_t hashFuncLen(char* str){
    return strlen(str);
}
uint32_t hashFuncSum(char* str){
    uint32_t sum = 0;
    while(*str){
        sum += *((unsigned char*)str);
        str++;
    }
    return sum;
}
uint32_t hashFuncXrol(char* str){
    uint32_t res = 0;
    while(*str){
        res = ((res << 1) | (res >> (sizeof(res)*8 - 1))) ^ *((unsigned char*)str);
        str++;
    }
    return res;
}
uint32_t hashFuncXror(char* str){
    uint32_t res = 0;
    while(*str){
        res = ((res >> 1) | (res << (sizeof(res)*8 - 1))) ^ *((unsigned char*)str);
        str++;
    }
    return res;
}
uint32_t hashFuncGnu(char* str){
    uint32_t res = 5381;
    while(*str){
        res = (res * 33) + *((unsigned char*)str);
        str++;
    }
    return res;
}

int main(){
    FILE* input_file = fopen("input.txt", "r");
    if (!input_file) {
        fprintf(stderr, "can not open file\n");
        return 1;
    }

    String input_data = readFile(input_file);
    if(input_data.length == 0){
        fprintf(stderr, "file read error\n");
    }
    fclose(input_file);

    for(int i = 0; i < input_data.length; i++){
        if(!isgraph(input_data.chars[i])){ // replace all separators with spaces
            input_data.chars[i] = ' ';
        }
    }
    size_t input_word_cnt = 0;
    String* input_words = split(input_data, &input_word_cnt, ' ');
    if(!input_words){
        fprintf(stderr, "split error\n");
        return 1;
    }
    printf("Loading done: %d lines %d chars\n", input_word_cnt, input_data.length);
    
    uint32_t (*(hash_functions[]))(char*) = {hashFuncFirst, hashFuncLen, hashFuncSum, hashFuncXrol, hashFuncXror, hashFuncGnu};
    char* hashf_names[]                   = {"first chr", "length", "sum", "rol + xor", "ror + xor", "GNU hash"};
    const int hash_func_cnt = 6;
    HashTable hashtable = {};
    hashTableCtor(&hashtable, nullptr);

    FILE* out_file = fopen("pop_data.txt", "w");


    for(int i = 0; i < hash_func_cnt; i++){
        hashtable.hash_func = hash_functions[i];
        fprintf(stderr, "Testing hashf %s.. ", hashf_names[i]);

        for(int j = 0; j < input_word_cnt; j++){
            hashTableAdd(&hashtable, input_words[j].chars);
        }
        fprintf(stderr, "DONE. saving...");
        fputs(hashf_names[i], out_file);
        for(int j = 0; j < HASH_ARR_SIZE; j++){
            fprintf(out_file, "\t %d ", hashtable.buckets[j].size);
        }
        fprintf(out_file, "\n");
        fprintf(stderr, "DONE \n");
        hashTableClear(&hashtable);
    }
    fclose(out_file);
    fprintf(stderr, "COMPLETE\n");
    return 0;
}
