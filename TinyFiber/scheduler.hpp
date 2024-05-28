#pragma once

#include "boost/asio.hpp"

#include <vector>
#include <thread>
#include <functional>

namespace tinyfiber {

using namespace boost;

// Fixed-size pool of threads
// Executes submitted tasks in pooled threads
class ThreadPool {
 public:
  using Task = std::function<void()>;

  ThreadPool(size_t thread_count);
  ~ThreadPool();

  // Submit task for execution
  // Submitted task will be scheduled to run in one of the worker threads
  void Submit(Task task);

  // Submit continuation of current task for execution
  void SubmitContinuation(Task cont);

  // Locate current thread pool from task
  static ThreadPool* Current();

  // Wait for all tasks in the pool to complete and stop worker threads
  void Join();

 private:
  void StartWorkerThreads(size_t thread_count);
  void Work();

 private:
  // Use io_context as task queue
  asio::io_context io_context_;
  // Some magic for graceful shutdown
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  // Worker threads
  std::vector<std::thread> workers_;
  bool joined_{false};
};

}  // namespace tinyfiber
