#include "panic.hpp"

#include <iostream>


namespace detail {
	void Panic(const std::string& error) {
		std::cerr << error << std::endl;
		std::abort();
	}
}  // namespace detail

