#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "lib/String.h"
#include "lib/file_read.h"
#include "hashtable.h"

void testHTSearch(HashTable* hashtable, String* test_data, int test_data_size){
    volatile int r = 0;
    for(int i = 0; i < 5000000; i++){
        int rand_i = rand() % test_data_size;
        if (test_data[rand_i].length != 0){
            //printf("# %d %s\n", rand_i, test_data[rand_i].chars);
            r = hashTableFind(hashtable, test_data[rand_i].chars);
        }
        else{
            i--;
        }
    }
}

uint32_t hashFuncCrc(char* str){
    uint32_t res = 5381;
    uint16_t chr = *(uint16_t*)str;
    uint16_t oldchr = 1;
    //beware the big endian
    while ((chr & 0xFF) && oldchr)
    { // stop if \0 was used last loop as second chr OR if it is current first chr
        str += 2;
        oldchr = chr > 0xFF;
        res = _mm_crc32_u16(res, chr);
        chr = *(uint16_t*)str;
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
    printf("Loading done: %ld lines %ld chars\n", input_word_cnt, input_data.length);

    HashTable hashtable = {};
    hashTableCtor(&hashtable, hashFuncCrc);

    for(int i = 0; i < input_word_cnt; i++){
        //if(input_words[i].length > 0)
        //    printf("%s\n", input_words[i].chars);
        hashTableAdd(&hashtable, input_words[i].chars);
    }

    testHTSearch(&hashtable, input_words, input_word_cnt);

    return 0;
}
