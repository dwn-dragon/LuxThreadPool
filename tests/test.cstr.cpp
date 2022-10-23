#include <iostream>
#include <lux_thread_pool.hpp>

int main(int argc, char const *argv[]) {
	try {
		lux::thread_pool tpool;
	} catch (...) {
		//	
		return 1;
	}
	
	return 0;
}
