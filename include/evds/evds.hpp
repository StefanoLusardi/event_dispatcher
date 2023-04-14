#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <type_traits>
#include <map>
#include <any>
#include <queue>
#include <future>

template <typename RetType, typename... Args>
struct function_traits_helper
{
    using ReturnT = RetType;
    using FunctionT = RetType(Args...);
    using FunctionWrapperT = std::function<FunctionT>;
};

template <typename T>
struct function_traits;

template <typename RetType, typename... Args>
struct function_traits<RetType(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename RetType, typename... Args>
struct function_traits<RetType (*)(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename RetType, typename... Args>
struct function_traits<RetType (&)(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename CType, typename RetType, typename... Args>
struct function_traits<RetType (CType::*)(Args...)> : function_traits_helper<RetType, Args...>
{
};

template <typename CType, typename RetType, typename... Args>
struct function_traits<RetType (CType::*)(Args...) const> : function_traits_helper<RetType, Args...>
{
};

template <typename T>
struct function_traits : function_traits<decltype(&T::operator())>
{
};

//------------------------------------------------------------------------------------------------

class event_dispatcher
{
public:
    explicit event_dispatcher()
        : _is_running{false}
    {
    }

    event_dispatcher(event_dispatcher&) = delete;
    event_dispatcher(const event_dispatcher&) = delete;
    event_dispatcher(event_dispatcher&&) noexcept = delete;
    event_dispatcher& operator=(const event_dispatcher&) = delete;
    event_dispatcher& operator=(event_dispatcher&&) = delete;
    ~event_dispatcher() { stop(); };

    void start()
    {
        std::promise<void> thread_started_notifier;
        std::future<void> thread_started_watcher = thread_started_notifier.get_future();
        _dispatcher_thread = std::thread([this, &thread_started_notifier]{dispatcher(std::move(thread_started_notifier));});
        thread_started_watcher.wait();
    }

    void stop()
    {
        {
            std::unique_lock<std::mutex> lock(_handlers_mutex);
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
    }

    template <typename F>
    auto add_event_handler(const std::string& event_name, F&& event_handler) -> bool
    {
        std::unique_lock lock(_handlers_mutex);

        using HandlerT = typename function_traits<F>::FunctionWrapperT;
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
    auto push_event(const std::string& event_name, Args&&... args) -> bool
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
    void dispatcher(std::promise<void>&& thread_started_notifier)
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

private:
    std::atomic_bool _is_running;
    std::thread _dispatcher_thread;
    std::map<std::string, std::any> _handlers;
    std::queue<std::function<void()>> _handlers_queue;    
    std::mutex _handlers_mutex;
    std::condition_variable _handlers_cv;
};
