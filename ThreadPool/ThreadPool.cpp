#include "ThreadPool.hpp"
#include <utility>


ThreadPool::ThreadPool(int thread_num) : thread_vec(thread_num) {
    sem = dispatch_semaphore_create(0);
    sem_q = dispatch_semaphore_create(0);
    for (auto& thread : thread_vec) {
        thread = std::thread(&ThreadPool::wait_for_task, this);
    }
}


ThreadPool::~ThreadPool() {
    for (auto& thread : thread_vec) {
        thread.join();
    }
    dispatch_release(sem);
}

void ThreadPool::wait() {
    while(!task_queue.empty()) {
    }
    stop_pool();
}

void ThreadPool::stop_pool () {
    is_active = false;
    for (auto& thread : thread_vec) {
        thread.join();
    }
}

void ThreadPool::wait_for_task() {
    while(is_active) {
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
        printf("locking wait_for_task\n");
        mut.lock();
        std::pair<fptr, void*> p;
        if (!task_queue.empty()) {
            printf("take task\n");
            p = std::move(task_queue.front());
            printf("pop task\n");
            task_queue.pop();
        }
        mut.unlock();
        p.first(p.second);
    }
}


void ThreadPool::push_task(void (*func)(void*), void* args) {
    printf("locking q\n");
    mut_push.lock();
    printf("pushing...\n");
    std::pair<fptr, void*> task(func, args);
    task_queue.push(task);
    mut_push.unlock();
    dispatch_semaphore_signal(sem);
}
