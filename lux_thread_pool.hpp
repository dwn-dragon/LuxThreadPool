#pragma once

#ifndef LUX_INCLUDE_THREAD_POOL
#define LUX_INCLUDE_THREAD_POOL

#undef LUX_CURR_INLINE

//
//	Header
//

#include <atomic>
#include <thread>
#include <future>
#include <functional>

namespace lux
{

	/**
     * @brief Length type for threads
     * 
     */
	using tsize_t = decltype(std::thread::hardware_concurrency());
	/**
     * @brief State type of a thread pool
     * 
     */
	using tstate_t = uint8_t;

	/**
     * @brief Valid states
     * 
     */
	enum : tstate_t
	{
		TS_RUNNING,
		TS_PAUSED,
		TS_TERMINATED
	};

	class thread_pool
	{
		struct NODE;
	public:
		/**
         * @brief Used to store the general state of a thread pool.
         * It has both the size of the underlying queue and the state. It allows a more reactive response from the workers by waiting both on the state and the number of tasks at the same time.
         * 
         */
        struct data_t
        {
            /**
             * @brief The size of the underlying queue of the thread pool.
             * 
             */
            size_t size;
            /**
             * @brief The state of the thread pool.
             * 
             */
            tstate_t state;
        };

		~thread_pool();

		/**
		 * @brief Constructs a new lock free thread pool object.
		 * 
		 * @param num The number of workers
		 */
		thread_pool(const tsize_t num = std::thread::hardware_concurrency());

	private:
		void _extract_run();
		void _worker_main(tsize_t pos, std::stop_token stoken);

	public:
		/**
		 * @brief Retuns the number of workers.
		 * 
		 * @return tsize_t The number of workers
		 */
		tsize_t workers() const noexcept;
		
		/**
		 * @brief Returns the general state of the queue.
		 * It has both the current state and the number of queued tasks.
		 * 
		 * @return data_t The general state of the queue
		 */
		data_t data() const noexcept;
		/**
		 * @brief Returns the current state of the thread pool.
		 * 
		 * @return tstate_t The current state of the thread pool
		 */
		tstate_t state() const noexcept;

		/**
		 * @brief Retuns the running tasks. The thread pool might be running no tasks, but have some queued.
		 * 
		 * @return tsize_t The number of tasks running
		 */
		size_t running_tasks() const noexcept;
		/**
		 * @brief Retuns the queued tasks.
		 * 
		 * @return size_t The number of tasks in the queue
		 */
		size_t queued_tasks() const noexcept;

	private:
		void _insert(std::shared_ptr<class vTask>&& task);

	public:
		/**
		 * @brief 
		 * 
		 * @param task 
		 * @return true 
		 * @return false 
		 */
		bool push_task(std::shared_ptr<class vTask> task);
		/**
		 * @brief Submits a callable with its args for execution.
		 * 
		 * @tparam Fn Callable type
		 * @tparam Args Arguments types
		 * @param fn Callable object
		 * @param args Arguments objects
		 * @return std::future<std::invoke_result_t<Fn, Args...>> The future object to fetch the result
		 */
		template< class Fn, class... Args >
		std::future<std::invoke_result_t<Fn, Args...>> submit(Fn&& fn, Args&&... args);

		/**
         * @brief Pauses the thread pool. Any task that is already running will not be paused, but the workers won't extract any other task from the underlying queue.
         * This method will fail if the thread pool is already paused or terminated.
         * 
         * @return true Only when the thread pool has been paused
         * @return false Otherwise
         */
		bool pause() noexcept;
		/**
         * @brief Unpauses the thread pool. Any sleeping worker will be woke up and resume the extraction-execution operations.
         * This method will fail if the thread pool is already not paused or terminated.
         * 
         * @return true When the thread pool has been unpaused
         * @return false Otherwise
         */
		bool unpause() noexcept;

		/**
		 * @brief Waits for any task.
		 * If the thread pool state is TS_PAUSED, it will only wait for the running tasks. If the thread pool state is TS_TERMINATED instead, it won't wait for any task.
		 * 
		 */
		void wait_for_tasks() const noexcept;

	private:
		//	thread pool state
		std::atomic<data_t> _data;

		//	workers
		tsize_t _wrkc;
		std::unique_ptr<std::jthread[]> _wrks;

		//	queue
		std::atomic<NODE*> _head;
		std::atomic<NODE*> _tail;

		//	misc
		std::atomic<size_t> _running;
	};

	/**
	 * @brief Virtual class for thread_pool tasks
	 * 
	 */
	class vTask
	{
	public:
		virtual ~vTask() = default;
	protected:
		vTask() = default;

	public:
		/**
		 * @brief Executes the task. 
		 */
		virtual void operator()() noexcept = 0;
	};

	/**
	 * @brief Wraps a function with a future.
	 * 
	 * @tparam Ty Type of the future
	 */
	template< class Ty >
	class Task : public vTask
	{
	public:
		/**
		 * @brief Construct an empty Task object
		 * 
		 */
		Task() = default;
		/**
		 * @brief Construct a Task object wrapping the wanted function
		 * 
		 * @tparam Fn Callable type
		 * @tparam Args Callable's arguments types
		 * @param fn Callable object
		 * @param args Callable's arguments objects
		 */
		template< class Fn, class... Args >
		Task(Fn&& fn, Args&&... args);

		/**
		 * @brief Used to get a future bound to the Task
		 * 
		 * @return std::future<Ty> - Future bound to the Task's promise 
		 */
		std::future<Ty> future();
		/**
		 * @brief Runs the task. The task results can be fetched by using the bound future
		 */
		void operator()() noexcept override;

	private:
		std::promise<Ty> _prom;
		std::function<Ty()> _fn;
	};

}

//	
//	Template
//	

#undef LUX_CURR_INLINE
#define LUX_CURR_INLINE inline

//	Thread Pool
//	Templates
//

LUX_CURR_INLINE bool lux::thread_pool::push_task(std::shared_ptr<vTask> task) {
	//	checks if the task is valid
	if (!task)
		return false;
	//	inserts the task in the queue
	_insert(std::move(task));
	return true;
}
template< class Fn, class... Args >
LUX_CURR_INLINE std::future<std::invoke_result_t<Fn, Args...>> lux::thread_pool::submit(Fn&& fn, Args&&... args) {
	if (state() == TS_TERMINATED)
		throw std::runtime_error{ "thread pool has been terminated" };

	using res_type = std::invoke_result_t<Fn, Args...>;

	auto task = std::make_shared<lux::Task<res_type>>( std::forward<Fn>(fn), std::forward<Args>(args)... );
	auto ftr = task->future();

	_insert(std::move(task));
	return std::move(ftr);
}

//	Inline
//	

LUX_CURR_INLINE lux::tsize_t lux::thread_pool::workers() const noexcept {
	return _wrkc;
}
LUX_CURR_INLINE lux::thread_pool::data_t lux::thread_pool::data() const noexcept {
	return _data.load();
}
LUX_CURR_INLINE lux::tstate_t lux::thread_pool::state() const noexcept {
	return data().state;
}
LUX_CURR_INLINE size_t lux::thread_pool::running_tasks() const noexcept {
	return _running.load();
}
LUX_CURR_INLINE size_t lux::thread_pool::queued_tasks() const noexcept {
	return data().size;
}

//	Task
//	Templates
//	

template< class Ty >
template< class Fn, class... Args >
LUX_CURR_INLINE lux::Task<Ty>::Task(Fn&& fn, Args&&... args) 
	: _fn{ std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...) } {
}
template< class Ty >
LUX_CURR_INLINE std::future<Ty> lux::Task<Ty>::future() {
	return _prom.get_future();
}
template< class Ty >
LUX_CURR_INLINE void lux::Task<Ty>::operator()() noexcept {
	try {
		if constexpr (std::is_void_v<Ty>) {
			_fn();
			_prom.set_value();
		}
		else {
			_prom.set_value(_fn());
		}
	}
	catch(...) {
		try {
			_prom.set_exception(std::current_exception());
		}
		catch(...) {
			//	do something
		}
	}	
}

//
//	Source
//

#undef LUX_CURR_INLINE
#if defined( LUX_INLINE_SOURCE ) || defined( LUX_SOURCE )

#if defined( LUX_INLINE_SOURCE )
#define LUX_CURR_INLINE	inline
#else
#define LUX_CURR_INLINE 
#endif

struct lux::thread_pool::NODE
{
	std::atomic<NODE*> _next;
	std::shared_ptr<lux::vTask> _data;
};

LUX_CURR_INLINE lux::thread_pool::~thread_pool() {
	//	updates the data
	data_t data = _data.load(), ndata{ data.size, TS_TERMINATED };
	while (!_data.compare_exchange_weak(data, ndata)) {
		ndata = { data.size, TS_TERMINATED };
	}

	//	notifies waiting workers
	_data.notify_all();

	//	closes the workers
	for	(tsize_t i = 0; i < _wrkc; ++i) {
		_wrks[i].request_stop();
		if (_wrks[i].joinable())
			_wrks[i].join();
	}

	//	clears the queue
	auto curr = _head.load(std::memory_order_relaxed);
	while (curr) {
		auto tmp = curr->_next.load(std::memory_order_relaxed);
		delete curr;
		curr = tmp;
	}
}
LUX_CURR_INLINE lux::thread_pool::thread_pool(const tsize_t num) 
	: _data{{ 0, TS_RUNNING }}, _wrkc{ num }, _running{ 0 } {
	//	inits the queue
	NODE* nn = new NODE{ nullptr, nullptr };
	_head.store(nn, std::memory_order_relaxed);
	_tail.store(nn, std::memory_order_relaxed);

	//	starts the workers
	_wrks = std::make_unique<std::jthread[]>(_wrkc);
	for (tsize_t i = 0; i < _wrkc; ++i)
		_wrks[i] = std::jthread{ std::bind_front(&lux::thread_pool::_worker_main, this, i) };
}

LUX_CURR_INLINE void lux::thread_pool::_extract_run() {
	//	EXTRACTION
	//	loads the head
	auto cn = _head.load();
	//	extracts a node
	while (true) {
		//	checks if a worker has already extracted the head
		if (cn == nullptr) {
			//	head is empty
			//	reloads head
			cn = _head.load();
		}
		else {
			//	head is valid
			//	tries to lock the head
			if (_head.compare_exchange_weak(cn, nullptr)) {
				//	replaces nullptr with the next node
				_head.store(cn->_next.load());
				//	leaves the cycle
				break;
			}
		}
	}

	//	EXECUTION
	//	the node is locked
	//	extracts the task
	auto task = std::move(cn->_data);
	//	frees the memory
	delete cn;
	//	runs the task
	if (task) (*task)();
}
LUX_CURR_INLINE void lux::thread_pool::_worker_main(tsize_t pos, std::stop_token stoken) {
	//	worker loop
	while (true) {
		//	data
		data_t data{ 0, TS_RUNNING };
		// waits for a task
		_data.wait(data);

		//	stop has been requested
		if (stoken.stop_requested())
			break;

		//	loads the actual data
		data = _data.load();
		//	handles the state
		switch (data.state)
		{
		//	thread pool has been terminated
		case TS_TERMINATED:
			return;

		//	thread pool has been paused
		case TS_PAUSED:
			_data.wait(data);
			break;

		//	thread pool is running
		case TS_RUNNING: {
			//	RESERVATION
			//	makes sure the queue has an element
			if (data.size == 0) {
				//	queue is empty
				//	do something
			}
			else {
				//	increases the working threads before reducing the size
				++_running;

				//	tries to reserve an element
				data_t ndata{ data.size - 1, TS_RUNNING };
				if (_data.compare_exchange_weak(data, ndata))
					//	an element is now reserved for the current worker
					_extract_run();
				//	sets worker as not running
				//	notifies when it's the last worker
				if (--_running == 0)
					_running.notify_all();
			}
		}
		default:
			break;
		}
	}
}
LUX_CURR_INLINE void lux::thread_pool::_insert(std::shared_ptr<vTask>&& task) {
	//	allocates new node
	NODE* nn = new NODE{ nullptr, nullptr };

	//	appends the new node
	NODE *null, *cn;
	do {
		null = nullptr;
		//	loads the current tail
		//	possible lock if tail is never updated
		cn = _tail.load();
		//	tries to append the new node
	} while (!cn->_next.compare_exchange_weak(null, nn));

	//	the node is now locked
	//	updates tail as first to allow more insertions
	_tail.store(nn);
	//	updates cn with the wanted value
	cn->_data = std::move(task);

	//	updates size as last
	data_t data = _data.load(), ndata{ data.size + 1, data.state };
	//	replaces data with new data
	while (!_data.compare_exchange_weak(data, ndata)) {
		//	recreates new data
		ndata = { data.size + 1, data.state };
	}

	//	notifies sleeping workers if not paused and queue was empty
	if (data.size == 0 && data.state != TS_PAUSED) {
		//	notifies 
		auto notc = _wrkc - running_tasks();
		//	notifies workers
		for (tsize_t i = 0; i < notc; ++i)
			_data.notify_one();
	}
}

LUX_CURR_INLINE bool lux::thread_pool::pause() noexcept {
	while (true) {
		auto data = _data.load();
		switch (data.state)
		{
		case TS_RUNNING:
			if (_data.compare_exchange_weak(data, { data.size, TS_PAUSED }))
				return true;
		
		default:
			return false;
		}
	}
}
LUX_CURR_INLINE bool lux::thread_pool::unpause() noexcept {
	while (true) {
		auto data = _data.load();
		switch (data.state)
		{
		case TS_PAUSED:
			if (_data.compare_exchange_weak(data, { data.size, TS_RUNNING })) {
				_data.notify_all();
				return true;
			}
		
		default:
			return false;
		}
	}
}
LUX_CURR_INLINE void lux::thread_pool::wait_for_tasks() const noexcept {
	//	thread pool has been terminated
	while (true) {
		//	gets running tasks count
		auto rt = running_tasks();
		if (rt == 0) {
			//	no running task
			auto data = _data.load();
			switch (data.state)
			{
			case TS_PAUSED:
			case TS_TERMINATED:
				return;
			
			default:
				if (data.size == 0)
					return;
			}
		}
		else {
			//	workers are running tasks
			//	waits for no running tasks
			_running.wait(rt);
		}
	}
}

#endif	//	Source guard
#endif	//	Include guard
