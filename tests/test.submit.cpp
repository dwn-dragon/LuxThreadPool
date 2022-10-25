#include <tests/test.misc.hpp>

std::atomic<size_t> count = 0;

void task_n0() {
	++count;
}
size_t task_v0() {
	return ++count;
}

void task_n1(size_t a) {
	count += a;
}
size_t task_v1(size_t a) {
	return count += a;
}

void task_n2(size_t a, size_t b) {
	count += (a + b);
}
size_t task_v2(size_t a, size_t b) {
	return count += (a + b);
}

int main(int argc, char const *argv[]) {
	size_t VAL1 = 3, VAL2 = 4, cnt = 0;
	lux::thread_pool tpool;

	tpool.submit(task_n0);
	tpool.wait_for_tasks();
	assert(++cnt, count.load());
	tpool.submit(task_v0);
	tpool.wait_for_tasks();
	assert(++cnt, count.load());

	tpool.submit(task_n1, VAL1);
	tpool.wait_for_tasks();
	assert(cnt += VAL1, count.load());
	tpool.submit(task_v1, VAL1);
	tpool.wait_for_tasks();
	assert(cnt += VAL1, count.load());

	tpool.submit(task_n2, VAL1, VAL2);
	tpool.wait_for_tasks();
	assert(cnt += (VAL1 + VAL2), count.load());
	tpool.submit(task_v2, VAL1, VAL2);
	tpool.wait_for_tasks();
	assert(cnt += (VAL1 + VAL2), count.load());

	return 0;
}
