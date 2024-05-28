#include "fiber.h"

#include <iostream>



void tinyfiber::Spawn(FiberRoutine routine, ThreadPool& thread_pool)
{

	//Coroutine coroutine(std::move(routine));
	
	

	/*const std::function<void()> task = [co, &task]() {
		co->Resume();
		std::cout << "Hi" << std::endl;
		if (!co->isCompleted()) {
			std::cout << " Hello" << std::endl;
			ThreadPool::Current()->Submit(task);
		}
	};*/

	Fiber fiber(std::make_shared<Coroutine>(std::move(routine)), thread_pool);

	thread_pool.Submit(fiber);
}

void tinyfiber::Spawn(FiberRoutine routine)
{
	Spawn(std::move(routine), *ThreadPool::Current());
}

void tinyfiber::Yieldd()
{
	Coroutine::Suspend();
}

tinyfiber::Fiber::Fiber(std::shared_ptr<Coroutine> coroutine, ThreadPool& pool)
	:coroutine_(std::move(coroutine)), tPool_(pool)
{
}

void tinyfiber::Fiber::operator()()
{
	coroutine_->Resume();
	if (!coroutine_->isCompleted()) {
		tPool_.Submit(Fiber(coroutine_, tPool_));
		//ThreadPool::Current()->Submit(Fiber(coroutine_));
	}
}

