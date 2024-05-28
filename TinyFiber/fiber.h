#pragma once

#include <functional>

#include "scheduler.hpp"
#include "Coroutine.h"

namespace tinyfiber {

	using FiberRoutine = std::function<void()>;

	// Spawn fiber inside provided thread pool
	void Spawn(FiberRoutine routine, ThreadPool& thread_pool);

	// Spawn fiber inside current thread pool
	void Spawn(FiberRoutine routine);

	void Yieldd();

	class Fiber {
	public: 
		Fiber(std::shared_ptr<Coroutine> coroutine, ThreadPool& pool);

		void operator()();

	private:
		std::shared_ptr<Coroutine> coroutine_;
		ThreadPool& tPool_;
	};

} // namespace tinyfiber