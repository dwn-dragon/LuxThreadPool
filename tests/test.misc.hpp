#include <lux_thread_pool.hpp>
#include <sstream>

class assert_error : public std::runtime_error
{
public:
	virtual ~assert_error() = default;

	assert_error(const char* msg) 
		: runtime_error{ msg } {
	}
	assert_error(std::string msg) 
		: runtime_error{ msg } {
	}
};

template< class T, class U >
inline void assert(T&& exp, U&& obt) {
	if (std::forward<T>(exp) == std::forward<U>(obt))
		return;
	
	std::stringstream ss;
	ss << "expected: " << std::forward<T>(exp) << " obtained: " << std::forward<U>(obt);
	throw assert_error{ ss.str() };
}
