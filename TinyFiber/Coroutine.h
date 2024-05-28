#pragma once

#include "stack.hpp"
#include "context.hpp"
#include "exception.h"

#include <functional>
#include <stdexcept>
#include <stack>

namespace tinyfiber {

struct CoroutineCompleted : public std::runtime_error
{
	CoroutineCompleted() : std::runtime_error("Coroutine completed") {}
};

struct NotInCoroutine: public std::runtime_error
{
	NotInCoroutine() : std::runtime_error("Not in coroutine") {}
};

class Coroutine;

void run(void* that);
void run_without_args();

class Coroutine
{
	using Routine = std::function<void()>;
public:
	Coroutine(Routine routine);

	// Transfers control to coroutine
	void Resume();

	// Suspends current coroutine and
    // transfers control back to caller
	static void Suspend();

	bool isCompleted() const;

	void RunUserRoutine();

	Coroutine(Coroutine&&) = default;
	Coroutine& operator=(Coroutine&&) = default;

private:
	Stack stack_;
	ExecutionContext callee_context_;
	ExecutionContext caller_context_;

	Routine routine_;
	std::exception_ptr exception_;
	bool complete_;

	friend void run(void* that);
	friend void run_without_args();
};



} // namespace tinyfiber


