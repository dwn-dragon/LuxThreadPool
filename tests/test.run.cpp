#include <tests/test.misc.hpp>

std::atomic<size_t> count = 0;
void incr() {
	++count;
}

int main(int argc, char const *argv[]) {
	if (argc < 2)
		throw std::bad_function_call{ };
	auto cycles = std::stoi(argv[1]);
	
	lux::thread_pool tpool;
	for (size_t i = 0; i < cycles; i++)
		tpool.submit(incr);
	tpool.wait_for_tasks();

	assert(cycles, count);
	return 0;
}
