#include "exception.h"

#include <stdexcept>
// memcpy
#include <cstring>


//TODO

namespace __cxxabiv1 {  // NOLINT

	//struct __cxa_eh_globals;  // NOLINT

	struct  __cxa_eh_globals {
		void    * caughtExceptions;
		unsigned int        uncaughtExceptions;
	};


	// NOLINTNEXTLINE
	extern "C" __cxa_eh_globals * __cxa_get_globals() noexcept;

}  // namespace __cxxabiv1

namespace __cxxabiv1 {
	extern "C" {
		static __cxa_eh_globals eh_globals;
		__cxa_eh_globals* __cxa_get_globals() noexcept { return &eh_globals; }
		//__cxa_eh_globals* __cxa_get_globals_fast() { return &eh_globals; }
	}
}


void tinyfiber::SwitchExceptionsContext(ExceptionsContext& from, ExceptionsContext& to)
{
	static constexpr size_t kStateSize = sizeof(ExceptionsContext);

	auto* this_thread_exceptions = __cxxabiv1::__cxa_get_globals();
	memcpy(from.exceptions_state_buf_, this_thread_exceptions, kStateSize);
	memcpy(this_thread_exceptions, to.exceptions_state_buf_, kStateSize);
}
