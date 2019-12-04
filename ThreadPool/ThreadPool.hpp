#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <dispatch/dispatch.h>
#include <string.h>
#include <errno.h>
#include <algorithm>

typedef void (*fptr)(void*);

class ThreadPool {
public:
    void push_task(void (*func)(void*), void* args);
    void wait();
    ThreadPool(int thread_num);
    ~ThreadPool();
private:
    void stop_pool();
    bool is_active;
    std::mutex mut;
    std::mutex mut_push;
    dispatch_semaphore_t sem;
    dispatch_semaphore_t sem_q;
    void wait_for_task();
    std::vector<std::thread> thread_vec;
    std::queue<std::pair<fptr, void*>> task_queue;
};
