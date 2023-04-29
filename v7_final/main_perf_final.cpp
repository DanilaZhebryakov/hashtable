#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "lib/String.h"
#include "lib/file_read.h"
#include "hashtable.h"
#include "config.h"

#ifdef DEBUG_MODE
    const int test_iterations_count = 5000000;
    extern bool is_test_running;
#else
    #ifdef PROFILING_MODE
        const int test_iterations_count = 5000000;
    #else
        const int test_iterations_count = 100000000;
    #endif
#endif

void testHTSearch(HashTable* hashtable, String* test_data, int test_data_size){
    #ifdef DEBUG_MODE
        is_test_running = true;
    #endif

    volatile int r = 0;

    long long tsc_sum = 0;
    for(int i = 0; i < test_iterations_count; i++){
        int rand_i = rand() % test_data_size;
        if (test_data[rand_i].length != 0){
            #ifndef PROFILING_MODE
            long long tsc_before;
            asm volatile ( "rdtsc\n"
                  "movl %%edx, %%ebx\n"
                  "shlq $32, %%rbx\n"
                  "orq %%rax, %%rbx\n"
                : "=b" (tsc_before)
                : 
                : "rax", "rdx"
                );
            #endif
            //printf("# %d %s\n", rand_i, test_data[rand_i].chars);
            r = hashTableFind(hashtable, test_data[rand_i].chars);
            #ifndef PROFILING_MODE
            long long tsc_after;
            asm volatile ( "rdtsc\n"
                "shlq $32, %%rdx\n"
                "orq %%rax, %%rdx\n"
                //"int $3\n"
                : "=d" (tsc_after)
                :
                : "rax"
                );
            tsc_sum += tsc_after - tsc_before;
            #endif
        }
        else{
            i--;
        }
    }
    printf("Cycles:%lld\n", tsc_sum);
    #ifdef DEBUG_MODE
        is_test_running = true;
    #endif
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

    for(size_t i = 0; i < input_data.length; i++){
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
    hashTableCtor(&hashtable);

    for(size_t i = 0; i < input_word_cnt; i++){
        //if(input_words[i].length > 0)
        //    printf("%s\n", input_words[i].chars);
        hashTableAdd(&hashtable, input_words[i].chars);
    }

    testHTSearch(&hashtable, input_words, input_word_cnt);

    return 0;
}