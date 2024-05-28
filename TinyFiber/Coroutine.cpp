#include "Coroutine.h"
#include <iostream>


namespace tinyfiber {
	static thread_local Coroutine* current = nullptr;
}


tinyfiber::Coroutine::Coroutine(Routine routine)
	: routine_(std::move(routine)), exception_(nullptr), complete_(false)
{
	stack_ = Stack::Allocate();
	//callee_context_.Setup(stack_.AsMemSpan(), run, this);
	callee_context_.Setup(stack_.AsMemSpan(), run_without_args);
}

void tinyfiber::Coroutine::Resume()
{
	if (isCompleted()) {
		throw CoroutineCompleted();
	}
	Coroutine* old_co = std::exchange(current, this);
	caller_context_.SwitchTo(callee_context_);
	current = old_co;
	if (exception_) {
		std::rethrow_exception(exception_);
	}
}

void tinyfiber::Coroutine::Suspend()
{
	if (current == nullptr) {
		throw NotInCoroutine();
	}
	current->callee_context_.SwitchTo(current->caller_context_);
}

bool tinyfiber::Coroutine::isCompleted() const
{
	return complete_;
}

void tinyfiber::Coroutine::RunUserRoutine()
{
	if (routine_) {
		try {
			routine_();
		}
		catch (std::runtime_error& e) {
			exception_ = std::make_exception_ptr(e);
		}
		catch (...) {
			exception_ = std::current_exception();
		}		
	}
}

void tinyfiber::run(void* that)
{
	Coroutine* co = (Coroutine*)that;
	if (co == current) {
		//std::cout << "!" << std::endl;
	}
	else {
		std::cout << "&" << std::endl;
	}

	try {
		co->routine_();
	}
	catch (...) {
		co->exception_ = std::current_exception();
	}



	co->complete_ = true; // защитить
	co->callee_context_.SwitchTo(co->caller_context_);
}


	void switch_f(tinyfiber::ExecutionContext& x, tinyfiber::ExecutionContext& y) {
		x.SwitchTo(y);
	}

void tinyfiber::run_without_args()
{
	if (current) {
		//std::cout << "!" << std::endl;

		current->RunUserRoutine();

		current->complete_ = true; // защитить (lock)
		current->callee_context_.SwitchTo(current->caller_context_);
		
	}
	else {
		std::cout << "&" << std::endl;
	}

}
