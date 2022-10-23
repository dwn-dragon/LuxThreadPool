#include <iostream>
#include <fstream>

#include <lux_thread_pool.hpp>

int fact(int val) {
	auto res = 1;
	for (size_t i = 2; i < val; i++)
		res = res * static_cast<int>(i);

	return res;
}

int main(int argc, char const *argv[]) {
	if (argc < 2)
		throw std::bad_function_call{ };
	auto cycles = std::stoi(argv[1]);
	
	lux::thread_pool tpool;
	for (size_t i = 0; i < cycles; i++)
		tpool.submit(fact, rand());
	tpool.wait_for_tasks();

	return 0;
}
