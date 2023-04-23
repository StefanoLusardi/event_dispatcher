#include <evds/event_dispatcher.hpp>

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

auto event_dispatcher::start() -> bool
{
    {
        std::scoped_lock<std::mutex> lock(_handlers_mutex);
        if(_is_running)
            return false;
    }

    std::promise<void> thread_started_notifier;
    std::future<void> thread_started_watcher = thread_started_notifier.get_future();
    _dispatcher_thread = std::thread([this, &thread_started_notifier]{ dispatcher(std::move(thread_started_notifier)); });
    thread_started_watcher.wait();

    return true;
}

auto event_dispatcher::stop() -> bool
{
    {
        std::scoped_lock<std::mutex> lock(_handlers_mutex);
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

    if (_dispatcher_thread.joinable())
        _dispatcher_thread.join();

    return true;
}

void event_dispatcher::dispatcher(std::promise<void>&& thread_started_notifier)
{
    _is_running = true;
    thread_started_notifier.set_value();

    while(_is_running)
    {
        std::unique_lock lock(_handlers_mutex);
        
        _handlers_cv.wait(lock, [this]{ return !_is_running || !_handlers_queue.empty(); });
        if (!_is_running)
            return;

        const auto event_handler = std::move(_handlers_queue.front());
        _handlers_queue.pop();
        lock.unlock();

        event_handler();
    }
}

}
