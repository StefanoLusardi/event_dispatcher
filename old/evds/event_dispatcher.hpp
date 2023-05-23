#pragma once

#include "function_traits.hpp"
#include "callable.hpp"

#include <any>
#include <future>
#include <iostream>
#include <map>
#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <optional>

namespace evds
{
class event_dispatcher
{
    template<typename... Args> void DEBUG_TYPE();

public:
    explicit event_dispatcher() noexcept;
    ~event_dispatcher();

    event_dispatcher(event_dispatcher&) = delete;
    event_dispatcher(const event_dispatcher&) = delete;
    event_dispatcher(event_dispatcher&&) noexcept = delete;
    event_dispatcher& operator=(const event_dispatcher&) = delete;
    event_dispatcher& operator=(event_dispatcher&&) = delete;

    auto start(const unsigned int num_threads = 1) -> bool;
    auto stop() -> bool;

    template <typename HandlerT>
    auto get_event_handler_id(const std::string& event_name)
    {
        auto id = std::move(event_name + std::to_string(typeid(HandlerT).hash_code()));
        //std::cout << id << std::endl;
        return id;
    }

    template <typename F>
    auto add_event_handler(const std::string& event_name, F&& event_handler) -> bool
    {
        using HandlerT = typename evds::details::function_traits<F>::wrapper_t;
        const auto event_handler_id = get_event_handler_id<HandlerT>(event_name);

        std::unique_lock handlers_lock(_handlers_mutex);
        auto&&[event_handler_iterator, is_inserted] = _handlers.emplace(std::move(event_handler_id), std::forward<HandlerT>(event_handler));
        (void)event_handler_iterator;
        return is_inserted;
    }

    template <typename RetType=void, typename... Args>
    auto remove_event_handler(const std::string& event_name) -> bool
    {
        using HandlerT = typename std::function<RetType(std::decay_t<Args>...)>;
        const auto event_handler_id = get_event_handler_id<HandlerT>(event_name); 

        std::unique_lock handlers_lock(_handlers_mutex);
        if (!_handlers.contains(event_handler_id))
            return false;
        
        _handlers.erase(event_handler_id);
        return true;
    }

    template <typename RetType, typename... Args>
    auto push_event(const std::string& event_name, Args... args) -> std::optional<std::future<RetType>>
    {
        using HandlerT = typename std::function<RetType(std::decay_t<Args>...)>;
        const auto event_handler_id = get_event_handler_id<HandlerT>(event_name); 

        std::unique_lock handlers_lock(_handlers_mutex);
        if (!_handlers.contains(event_handler_id))
            return std::nullopt;

        std::packaged_task<RetType()> event_handler_call([event_handler = std::any_cast<HandlerT>(_handlers.at(event_handler_id)), ... args = std::forward<std::decay_t<Args>>(args)]
        {
            return std::invoke(event_handler, args...);
        });

        std::future<RetType> future = event_handler_call.get_future();
        _handlers_queue.emplace(std::move(event_handler_call));

        handlers_lock.unlock();        
        _handlers_cv.notify_one();

        return future;
    }

protected:
    void dispatcher_worker();

private:
    std::atomic_bool _is_running;
    std::vector<std::jthread> _dispatcher_threads;
    std::map<std::string, std::any> _handlers;
    std::queue<evds::details::callable> _handlers_queue;    
    std::mutex _handlers_mutex;
    std::condition_variable _handlers_cv;
};

}