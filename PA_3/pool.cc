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
    std::lock_guard<std::mutex> lock(mtx); 
    if (done) { //if the task was already completed, we can't submit it to the queue
        std::cerr << "Cannot added task to queue\n";
        return;
    }
    task->name = name; //set the name of the task as the passed in one
    queue.push_back(task); //add it to the queue
    std::cout << "Added task " << name <<"\n"; 
    num_tasks_unserviced++; //increment counter
}

void ThreadPool::run_thread() {
    while (true) {
        Task* next_task = nullptr; //temp next task
       
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (done && queue.empty()) { //if we're done and the queue is empty just break
                break;
            }
            if (queue.empty()) { //if the queue is empty, we just move on
                continue;
            } else { //if it isn't we take off the front of the queue and set it equal to our next task and set its running status to true
                next_task = queue.front();
                queue.erase(queue.begin());
                next_task->running = true;
            }
        }

        std::cout << "Started task " << next_task->name << "\n";
        next_task->Run();
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (num_tasks_unserviced > 0) {
                --num_tasks_unserviced;
            }
        }
        std::cout << "Finished task " << next_task->name << "\n";
        next_task->running = false;
        delete next_task; //delete our temp task
    }
}

void ThreadPool::remove_task(Task *t) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto it = queue.begin(); it != queue.end();) { 
        if (*it == t) {
            delete *it;
            queue.erase(it);
            if (num_tasks_unserviced > 0) --num_tasks_unserviced;
            return;
        }
        ++it;
    }
}

void ThreadPool::Stop() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        done = true; 
    }
    std::cout << "Called Stop()\n";
    for (std::thread *t: threads) {
        std::cout << "Stopping threads\n";
        if (t && t->joinable()) {
            t->join(); //join the threads if they are joinable
        }
    }
}
