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
    using handler_call_t = std::function<void()>;

public:
    explicit event_dispatcher() noexcept;
    ~event_dispatcher();

    event_dispatcher(event_dispatcher&) = delete;
    event_dispatcher(const event_dispatcher&) = delete;
    event_dispatcher(event_dispatcher&&) noexcept = delete;
    event_dispatcher& operator=(const event_dispatcher&) = delete;
    event_dispatcher& operator=(event_dispatcher&&) = delete;

    auto start() -> bool;
    auto stop() -> bool;

    template <typename HandlerT>
    auto get_event_handler_id(const std::string& event_name)
    {
        return std::move(event_name + std::to_string(typeid(HandlerT).hash_code()));
    }

    template <typename F>
    auto add_event_handler(const std::string& event_name, F&& event_handler) -> bool
    {
        // decltype(&F::operator()) d;
        // static_assert(std::is_same_v<typename evds::details::function_traits<F>::return_t, void>, "event handler must return void");

        std::unique_lock handlers_lock(_handlers_mutex);

        using HandlerT = typename evds::details::function_traits<F>::wrapper_t;
        const auto event_handler_id = get_event_handler_id<HandlerT>(event_name); 
        auto&&[event_handler_iterator, is_inserted] = _handlers.emplace(std::move(event_handler_id), std::forward<HandlerT>(event_handler));
        (void)event_handler_iterator;
        return is_inserted;
    }

    template <typename RetType=void, typename... Args>
    auto remove_event_handler(const std::string& event_name) -> bool
    {
        std::unique_lock handlers_lock(_handlers_mutex);

        using HandlerT = typename std::function<RetType(Args...)>;
        const auto event_handler_id = get_event_handler_id<HandlerT>(event_name); 
        if (!_handlers.contains(event_handler_id))
            return false;
        
        _handlers.erase(event_handler_id);
        return true;
    }

    template <typename RetType=void, typename... Args>
    auto push_event(const std::string& event_name, Args... args) -> bool
    {
        std::unique_lock handlers_lock(_handlers_mutex);

        using HandlerT = typename std::function<void(Args...)>;
        const auto event_handler_id = get_event_handler_id<HandlerT>(event_name); 
        if (!_handlers.contains(event_handler_id))
            return false;

        const auto event_handler = _handlers.at(event_handler_id);
        const handler_call_t event_handler_call = [evt_h = std::any_cast<HandlerT>(event_handler), ... args = std::forward<Args>(args)]
        {
            return std::invoke(evt_h, args...);
        };

        _handlers_queue.emplace(std::move(event_handler_call));
        handlers_lock.unlock();
        
        _handlers_cv.notify_one();
        return true;
    }

protected:
    void dispatcher(std::promise<void>&& thread_started_notifier);

private:
    std::atomic_bool _is_running;
    std::thread _dispatcher_thread;
    std::map<std::string, std::any> _handlers;
    std::queue<handler_call_t> _handlers_queue;    
    std::mutex _handlers_mutex;
    std::condition_variable _handlers_cv;
};

}