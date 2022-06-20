#pragma once

#include <deque>
#include <vector>
#include <thread>
#include <semaphore>
#include <mutex>

template<typename T, typename C>
class Worker {
  std::counting_semaphore<> semaphore{0};
  std::vector<std::thread> threads;
  std::deque<T> queue;
  std::mutex qMutex;
  C callback;
  std::atomic<bool> running{true};

  void worker() {
    while (running || !queue.empty()) {
      semaphore.acquire();
      qMutex.lock();
      if (!queue.empty()) {
        T t = queue.front();
        queue.pop_front();
        qMutex.unlock();
        callback(t);
      }
    }
  }

public:
  Worker(C callback(T)) : callback(callback) {
    threads.resize(std::thread::hardware_concurrency(), this->worker);
  }

  void push(T t) {
    queue.push_back(t);
    semaphore.release();
  }

  void run() {
    running = true;
  }

  void join() {
    running = false;
    for (auto &t : threads) {
      semaphore.release();
      t.join();
    }
  }

  ~Worker() {
    queue.clear();
    running = false;
    for (auto &t : threads) {
      semaphore.release();
      t.join();
    }
  }
};