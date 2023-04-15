#pragma once

#include "function_traits.hpp"

#include <any>
#include <future>
#include <map>
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>

namespace evds
{
class event_dispatcher
{
public:
    explicit event_dispatcher() noexcept;
    ~event_dispatcher() noexcept;

    event_dispatcher(event_dispatcher&) = delete;
    event_dispatcher(const event_dispatcher&) = delete;
    event_dispatcher(event_dispatcher&&) noexcept = delete;
    event_dispatcher& operator=(const event_dispatcher&) = delete;
    event_dispatcher& operator=(event_dispatcher&&) = delete;

    auto start() -> bool;
    auto stop() -> bool;

    template <typename F>
    auto add_event_handler(const std::string& event_name, F&& event_handler) -> bool
    {
        std::unique_lock lock(_handlers_mutex);

        using HandlerT = typename evds::details::function_traits<F>::FunctionWrapperT;
        const auto event_handler_name = std::move(event_name + std::to_string(typeid(HandlerT).hash_code()));
        auto&&[event_handler_iterator, is_inserted] = _handlers.emplace(std::move(event_handler_name), std::forward<HandlerT>(event_handler));
        (void)event_handler_iterator;
        return is_inserted;
    }

    template <typename RetType=void, typename... Args>
    auto remove_event_handler(const std::string& event_name) -> bool
    {
        std::unique_lock lock(_handlers_mutex);

        using HandlerT = typename std::function<RetType(Args...)>;
        const auto event_handler_name = std::move(event_name + std::to_string(typeid(HandlerT).hash_code()));
        if (!_handlers.contains(event_handler_name))
            return false;
        
        _handlers.erase(event_handler_name);
        return true;
    }

    template <typename RetType=void, typename... Args>
    auto push_event(const std::string& event_name, Args... args) -> bool
    {
        std::unique_lock lock(_handlers_mutex);

        using HandlerT = typename std::function<RetType(Args...)>;
        const auto event_handler_name = std::move(event_name + std::to_string(typeid(HandlerT).hash_code()));
        if (!_handlers.contains(event_handler_name))
            return false;

        auto event_handler = _handlers.at(event_handler_name);
        std::function<void()> event_handler_call = [this, evt_h = std::any_cast<HandlerT>(event_handler), ... args = std::forward<Args>(args)]
        {
            return std::invoke(evt_h, args...);
        };

        _handlers_queue.emplace(std::move(event_handler_call));
        lock.unlock();
        
        _handlers_cv.notify_one();
        return true;
    }

protected:
    void dispatcher(std::promise<void>&& thread_started_notifier);

private:
    std::atomic_bool _is_running;
    std::thread _dispatcher_thread;
    std::map<std::string, std::any> _handlers;
    std::queue<std::function<void()>> _handlers_queue;    
    std::mutex _handlers_mutex;
    std::condition_variable _handlers_cv;
};

}