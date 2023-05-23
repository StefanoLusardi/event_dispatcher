#include <evds/event_dispatcher.hpp>
#include <barrier>

namespace evds
{
event_dispatcher::event_dispatcher() noexcept
    : _is_running{false}
{
}

event_dispatcher::~event_dispatcher()
{
    stop();
}

auto event_dispatcher::start(const unsigned int num_threads) -> bool
{
    {
        std::scoped_lock lock(_handlers_mutex);
        if(_is_running)
            return false;
    }

    auto dispatcher_threads_barrier_callback = [this]() noexcept
    {
        std::scoped_lock lock(_handlers_mutex);
        _is_running = true;
    };

    const auto thread_count = std::clamp(num_threads, 1u, std::thread::hardware_concurrency());
    _dispatcher_threads.reserve(thread_count);

    using barrier_t = std::barrier<decltype(dispatcher_threads_barrier_callback)>;
    auto dispatcher_threads_barrier = std::make_shared<barrier_t>(thread_count + 1, dispatcher_threads_barrier_callback);
    const auto dispatcher_thread = [this, &dispatcher_threads_barrier]() {
        dispatcher_threads_barrier->arrive_and_wait();
        dispatcher_worker();
    };

    for (auto i = 0; i < thread_count; ++i)
        _dispatcher_threads.emplace_back(dispatcher_thread);

    dispatcher_threads_barrier->arrive_and_wait();
    return true;
}

auto event_dispatcher::stop() -> bool
{
    {
        std::scoped_lock lock(_handlers_mutex);
        if(!_is_running)
            return false;

        _is_running = false;
    }

    _handlers_cv.notify_all();
    
    {
        std::scoped_lock lock(_handlers_mutex);
        _handlers.clear();
        _handlers_queue = {};
    }

    for (auto&& thread : _dispatcher_threads)
    {
        if (thread.joinable())
            thread.join();
    }
        
    return true;
}

void event_dispatcher::dispatcher_worker()
{
    while(_is_running)
    {
        std::unique_lock lock(_handlers_mutex);
        
        _handlers_cv.wait(lock, [this]{ return !_handlers_queue.empty() || !_is_running; });
        if (!_is_running)
            return;

        const auto event_handler = std::move(_handlers_queue.front());
        _handlers_queue.pop();
        lock.unlock();

        event_handler();
    }
}

}
