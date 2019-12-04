#include <cstdlib>
#include <algorithm>
#include <unistd.h>
#include <condition_variable>
#include "ThreadPool.hpp"


struct Argument {
    unsigned long int* array;
    size_t first_pos;
    size_t second_pos;
    bool is_greater;
};

struct ArgSort {
    unsigned long int* array;
    size_t size;
    ThreadPool* thread_pool;
    bool is_greater;
    std::mutex* m;
    std::condition_variable* c;
    bool is_wait;
};

struct ArgMerge {
    
};

void notify_or_wait(bool is_wait, std::condition_variable& c, std::mutex& m) {
    if (is_wait) {
        std::unique_lock<std::mutex> lock(m);
        c.wait(lock);
    } else {
        c.notify_one();
    }
}

void compare_and_swap(void* args) {
    unsigned long int* array = ((Argument*)args)->array;
    size_t first_pos = ((Argument*)args)->first_pos;
    size_t second_pos = ((Argument*)args)->second_pos;
    bool is_greater = ((Argument*)args)->is_greater;
    if (is_greater == (*(array + first_pos) > *(array + second_pos))) {
        std::swap(*(array + first_pos), *(array + second_pos));
    }
}


void bitonic_merge(void* arg) {
    /* Unpack arg */
    unsigned long int* array = ((ArgSort*)arg)->array;
    size_t size = ((ArgSort*)arg)->size;
    ThreadPool* thread_pool = ((ArgSort*)arg)->thread_pool;
    bool is_greater = ((ArgSort*)arg)->is_greater;

    if (size > 1) {
        /* Define sizes of subarrays and interval for compare_and_swap */
        size_t interval = size / 2;
        size_t left_size = size / 2;
        size_t right_size = size - left_size;
        Argument* args = (Argument*)malloc(sizeof(Argument));
        args->array = array;
        
        /* Compare and swap, this one should be parallel */
        for (int i = 0; i < interval; i++) {
            args->first_pos = i;
            args->second_pos = i + interval;
            args->is_greater = is_greater;
            compare_and_swap((void*)args);
        }
        
        /* Make an arg for left subarray and merge */
        ArgSort* narg = (ArgSort*)malloc(sizeof(ArgSort));
        narg->array = array;
        narg->size = left_size;
        narg->thread_pool = thread_pool;
        narg->is_greater = is_greater;

        bitonic_merge(narg);
        
        /* Make an arg for right subarray and merge */
        ArgSort* nnarg = (ArgSort*)malloc(sizeof(ArgSort));
        nnarg->array = array + left_size;
        nnarg->size = right_size;
        nnarg->thread_pool = thread_pool;
        nnarg->is_greater = is_greater;

        bitonic_merge(nnarg);

        free(narg);
        free(nnarg);
        free(args);
    }
}


void bitonic_sort_parallel(void* arg) {
    /* Unpack arg */
    unsigned long int* array = ((ArgSort*)arg)->array;
    size_t size = ((ArgSort*)arg)->size;
    ThreadPool* thread_pool = ((ArgSort*)arg)->thread_pool;
    bool is_greater = ((ArgSort*)arg)->is_greater;
    bool is_wait = ((ArgSort*)arg)->is_wait;

    if (size > 1) {
        /* Define sizes of subarrays */
        size_t left_size = size / 2;
        size_t right_size = size - left_size;
        
        /* Create mutex and cond var for next recursion level*/
        std::mutex m;
        std::condition_variable c;
        
        /* Make an arg for left subarray and push it to ThreadPool */
        ArgSort* narg = (ArgSort*)malloc(sizeof(ArgSort));
        narg->array = array;
        narg->size = left_size;
        narg->thread_pool = thread_pool;
        narg->is_greater = true;
        narg->m = &m;
        narg->c = &c;
        narg->is_wait = false;

        thread_pool->push_task(&bitonic_sort_parallel, narg);

        /* Make an arg for right sub array and execute it in a current thread */
        ArgSort* nnarg = (ArgSort*)malloc(sizeof(ArgSort));
        nnarg->array = array + left_size;
        nnarg->size = right_size;
        nnarg->thread_pool = thread_pool;
        nnarg->is_greater = false;
        narg->m = &m;
        narg->c = &c;
        narg->is_wait = true;

        bitonic_sort_parallel(nnarg);
        
        /* Make an arg for merge and execute it */
        ArgSort* mnnarg = (ArgSort*)malloc(sizeof(ArgSort));
        mnnarg->array = array;
        mnnarg->size = size;
        mnnarg->thread_pool = thread_pool;
        mnnarg->is_greater = is_greater;

        bitonic_merge(mnnarg);
        
        if (is_wait) {
            std::unique_lock<std::mutex> lock(*((ArgSort*)arg)->m);
            ((ArgSort*)arg)->c->wait(lock);
        } else {
            ((ArgSort*)arg)->c->notify_one();
        }

        free(narg);
        free(nnarg);
    }
}

int main(int argc, char const *argv[]) {
    int n = argc > 1 ? std::stoi(argv[1]) : 4;
    ThreadPool thread_pool(n);
    sleep(1);
    unsigned long int* array = (unsigned long int*)malloc(sizeof(unsigned long int) * n);
    for (int i = 0; i < n; i++) {
        array[i] = rand() % 10000;
    }
    printf("Initial array: ");
    for (int i = 0; i < n; i++) {
        printf("%ld ", array[i]);
    }
    printf("\n");

    ArgSort* arg = (ArgSort*)malloc(sizeof(ArgSort));
    arg->array = array;
    arg->size = n;
    arg->thread_pool = &thread_pool;
    arg->is_greater = false;
    std::mutex m;
    std::condition_variable c;
    arg->m = &m;
    arg->c = &c;
    long long start = __rdtsc();
    bitonic_sort_parallel(arg);
    thread_pool.wait();
    long long end = __rdtsc();
    printf("Time = %.6f\n", (end - start)/1000000000.);

    printf("Sorted array: ");
    for (int i = 0; i < n; i++) {
        printf("%ld ", array[i]);
    }
    printf("\n");
    free(arg);
    free(array);
    return 0;
}
