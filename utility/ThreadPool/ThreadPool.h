#pragma once
#include <future>
#include <thread>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <any>
namespace ix {
	namespace thread {
class ThreadPool
{
public:
	explicit ThreadPool(size_t thread_count);
	~ThreadPool();
    // 模板注册任务
    template<class F, class... Args>
    std::future<std::any> submit(F&& f, Args&&... args)
    {
        using return_type = std::invoke_result_t<F, Args...>;

        auto bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        std::function<return_type()> func = std::move(bound);

        // 2. promise<any> + future<any>
        auto promiseAny = std::make_shared<std::promise<std::any>>();
        auto futureAny = promiseAny->get_future();

        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            if (stop.load()) throw std::runtime_error("ThreadPool err");

            // 将人物压至任务队列
            tasks.emplace([func, promiseAny]() mutable {
                try {
                    if constexpr (std::is_void_v<return_type>) {
                        func();
                        promiseAny->set_value(std::any{});
                    }
                    else {
                        promiseAny->set_value(std::any(func()));
                    }
                }
                catch (...) {
                    promiseAny->set_exception(std::current_exception());
                }
                });
        }
        condition.notify_one();
        return futureAny;
    }

private:
	void worker_loop();
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex queue_mutex;
	std::condition_variable condition;
	std::atomic<bool> stop{false};
};

}
}


