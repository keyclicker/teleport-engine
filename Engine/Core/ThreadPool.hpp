#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace te {

/**
 * @brief Fixed set of worker threads that split numbered tasks
 *
 * The calling thread works too, so a pool of size 1 has no workers and
 * runs everything inline: that's the single-threaded web build.
 */
class ThreadPool {
private:
  std::vector<std::thread> workers;

  std::mutex mutex;
  std::condition_variable wake, finished;

  // The current batch, see run()
  const std::function<void(int)> *job = nullptr;
  int taskCount = 0;
  std::atomic<int> nextTask{0};

  // Workers still inside the current batch
  int busy = 0;

  // Bumped per batch, so workers can tell a new one from a spurious wake
  uint64_t batch = 0;
  bool stopping = false;

  void workerLoop();

  /**
   * @brief Take tasks until the batch runs out
   */
  void drain();

public:
  ThreadPool(const ThreadPool &rhs) = delete;
  ThreadPool &operator=(const ThreadPool &rhs) = delete;

  /**
   * @param threads Threads including the caller, see defaultSize()
   */
  explicit ThreadPool(int threads = defaultSize());

  ~ThreadPool();

  /**
   * @brief Hardware threads, or 1 where the build has no threads
   */
  static int defaultSize();

  /**
   * @brief Threads that run tasks, the caller included
   */
  int size() const { return int(workers.size()) + 1; }

  /**
   * @brief Call `task(i)` for every `i` in [0, count), in parallel, and
   *        return when all are done
   *
   * Tasks are handed out in order as threads free up, so many small
   * tasks balance better than one per thread.
   */
  void run(int count, const std::function<void(int)> &task);
};

} // namespace te
