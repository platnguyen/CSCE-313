#include "pool.h"
#include <mutex>
#include <iostream>

Task::Task() = default;
Task::~Task() = default;

ThreadPool::ThreadPool(int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(new std::thread(&ThreadPool::run_thread, this));
    }
}

ThreadPool::~ThreadPool() {
    for (std::thread *t: threads) {
        delete t;
    }
    threads.clear();

    for (Task *q: queue) {
        delete q;
    }
    queue.clear();
}

void ThreadPool::SubmitTask(const std::string &name, Task *task) {
    //TODO: Add task to queue, make sure to lock the queue
    mtx.lock();
    task->name = name;
    queue.push_back(task);
    num_tasks_unserviced++;
    mtx.unlock();
}

void ThreadPool::run_thread() {
    while (true) {
        Task* next_task = nullptr;
        {
            mtx.lock();
            //TODO1: if done and no tasks left, break
            if (done && queue.empty()) {
                mtx.unlock();
                break;
            }
            //TODO2: if no tasks left, continue
            if (queue.empty()) {
                mtx.unlock();
                continue; 
            }
            next_task = queue.front();
            queue.erase(queue.begin());
            next_task->running = true;
            mtx.unlock();
        }
        //TODO3: get task from queue, remove it from queue, and run it
        next_task->Run();
        next_task->running = false;
        //TODO4: delete task
        delete next_task;
    }
}

// Remove Task t from queue if it's there
void ThreadPool::remove_task(Task *t) {
    mtx.lock();
    for (auto it = queue.begin(); it != queue.end();) {
        if (*it == t) {
            queue.erase(it);
            num_tasks_unserviced--;
            mtx.unlock();
            return;
        }
        ++it;
    }
    mtx.unlock();
}

void ThreadPool::Stop() {
    //TODO: Delete threads, but remember to wait for them to finish first
    done = true;
    for (std::thread *t: threads) {
        t->join();
    }
}
