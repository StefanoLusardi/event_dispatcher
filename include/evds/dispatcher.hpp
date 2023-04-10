#pragma once

#include <deque>
#include <thread>
#include <functional>
#include <unordered_map>

namespace evds
{
class dispatcher
{
private:
    class handler
    {
    private:
        struct handler_base
        {
            virtual ~handler_base() = default;
            virtual void invoke() = 0;
        };

        template<typename FunctionType>
        struct handler_impl : public handler_base
        {
            explicit handler_impl(FunctionType&& f)
                : _func{ std::move(f) } 
            { 
                static_assert(std::is_invocable_v<decltype(f)>); 
            }
            void invoke() override { _func(); }
            FunctionType _func;
        };

    public:
        template<typename FunctionType>
        explicit handler(FunctionType&& f) 
            : _impl{ std::make_unique<handler_impl<FunctionType>>(std::move(f)) } 
        { }

        handler(handler&& other) noexcept 
            : _impl{ std::move(other._impl) } 
        { }

        handler(handler&) = delete;
        handler(const handler&) = delete;
        handler& operator=(const handler&) = delete;
        handler& operator=(handler&&) = delete;
        ~handler() = default;
        
        void invoke() { _impl->invoke(); }

    private:
        std::unique_ptr<handler_base> _impl;
    };

    // using handler = std::function<void()>;

public:
    explicit dispatcher()
    {
        _thread = std::thread([this]
        {
            while(true)
            {
                // auto event_handler = _events_queue.front();
                // handler->invoke();
            }
        });
    }

    dispatcher(dispatcher&) = delete;
    dispatcher(const dispatcher&) = delete;
    dispatcher(dispatcher&&) noexcept = delete;
    dispatcher& operator=(const dispatcher&) = delete;
    dispatcher& operator=(dispatcher&&) = delete;
    ~dispatcher() = default;

    template <typename event_handler_t, typename... Args>
    void publish(const char* topic, event_handler_t&& func, Args&&... args)
    {
        auto event_handler = _handlers.find(topic);
        auto params = std::make_tuple(std::forward<Args>(args)...);
        auto event = std::apply(event_handler, params);
        _events_queue.emplace_back(std::move(event));
    }

    template <typename event_handler_t>
    void subscribe(const char* topic, event_handler_t&& event_handler)
    {
        _handlers.emplace(topic, event_handler);
    }

    void unsubscribe(const char* topic) {}

private:
    std::thread _thread;
    std::deque<handler> _events_queue;
    std::unordered_map<const char*, std::weak_ptr<handler>> _handlers;
};

}
