#include <tests/test.misc.hpp>

int main(int argc, char const *argv[]) {
	try {
		lux::thread_pool tpool;
	} catch (...) {
		//	
		return 1;
	}
	
	return 0;
}
