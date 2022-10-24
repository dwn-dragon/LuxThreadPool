#include <iostream>
#include <chrono>

#include <config.h>
#include <lux_thread_pool.hpp>

std::mutex m;

size_t tot;
std::chrono::duration<double, std::milli> sum;

void _average() {
	auto str = std::chrono::steady_clock::now();

	//	
	auto val = rand();
	//
	//std::this_thread::sleep_for( std::chrono::duration<double, std::milli>{ 0.01 } );

	std::lock_guard lg{ m };
	sum += std::chrono::steady_clock::now() - str;
	++tot;
}

int main(int argc, char const *argv[]) {
	std::cout << "version: " << PROJECT_VER_MAJOR << "." << PROJECT_VER_MINOR << "." << PROJECT_VER_PATCH << "\n";
	std::cout << "hardware concurrency: " << std::thread::hardware_concurrency() << "\n";
	std::cout << "lock free: " << ((std::atomic<size_t>::is_always_lock_free && std::atomic<void*>::is_always_lock_free) ? "true" : "false") << "\n" << std::endl;

	//	multi threaded
	//

	tot = 0;
	sum = decltype(sum)::zero();

	lux::thread_pool tpool;

	std::cout << "starting..." << std::endl;
	auto str = std::chrono::steady_clock::now();
	
	for (size_t i = 0; i < 1000000; i++)
		tpool.submit( _average );
		
	tpool.wait_for_tasks();

	std::chrono::duration<double, std::milli> dt = std::chrono::steady_clock::now() - str;
	std::cout << "done\n" << std::endl;

	std::cout << "[scraps]\nqueued tasks: " << tpool.queued_tasks() << "\nrunning tasks: " << tpool.running_tasks() << "\n";
	std::cout << "[gross]\ntotal: " << (dt) << "\naverage: " << (dt / tot) << "\n";
	std::cout << "[net]\ntotal: " << (sum) << "\naverage: " << (sum / tot) << "\n";
	std::cout << "[overhead]\ntotal " << (dt - sum) << "\naverage: " << ((dt / tot) - (sum / tot)) << "\n" << std::endl;

	return 0;
}
