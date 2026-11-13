#include "ThreadPool.hpp"

#include <algorithm>

namespace te {

ThreadPool::ThreadPool(int threads) {
  for (int i = 1; i < threads; ++i) {
    workers.emplace_back([this] { workerLoop(); });
  }
}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard lock(mutex);
    stopping = true;
  }

  wake.notify_all();

  for (auto &worker : workers) {
    worker.join();
  }
}

int ThreadPool::defaultSize() {
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
  return 1;
#else
  // More than 16 rarely pays off for a few thousand columns
  return std::clamp(int(std::thread::hardware_concurrency()), 1, 16);
#endif
}

void ThreadPool::run(int count, const std::function<void(int)> &task) {
  if (workers.empty()) {
    for (int i = 0; i < count; ++i) task(i);
    return;
  }

  {
    std::lock_guard lock(mutex);
    job = &task;
    taskCount = count;
    nextTask = 0;
    busy = int(workers.size());
    ++batch;
  }

  wake.notify_all();
  drain();

  std::unique_lock lock(mutex);
  finished.wait(lock, [this] { return busy == 0; });
  job = nullptr;
}

void ThreadPool::drain() {
  for (int i = nextTask++; i < taskCount; i = nextTask++) {
    (*job)(i);
  }
}

void ThreadPool::workerLoop() {
  uint64_t seen = 0;

  while (true) {
    {
      std::unique_lock lock(mutex);
      wake.wait(lock, [&] { return stopping || batch != seen; });

      if (stopping) return;
      seen = batch;
    }

    drain();

    std::lock_guard lock(mutex);
    if (--busy == 0) finished.notify_one();
  }
}

} // namespace te
