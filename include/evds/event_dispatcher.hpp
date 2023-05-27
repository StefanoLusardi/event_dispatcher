#pragma once 

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <barrier>
#include <vector>

#include <evds/function_traits.hpp>

namespace evds
{
class event_dispatcher final
{
private:
    using event_t = std::function<void()>;

    struct base_handler_t
    {
        base_handler_t(unsigned long id) noexcept : id{id} {}
        virtual ~base_handler_t() noexcept {}
        base_handler_t(const base_handler_t&) = delete;
        base_handler_t(base_handler_t&&) noexcept = delete;
        base_handler_t& operator=(const base_handler_t&) = delete;
        base_handler_t& operator=(base_handler_t&&) = delete;

        const unsigned long id;
    };

    template <typename... Args>
    class handler_t : public base_handler_t
    {
    public:
        handler_t(unsigned long id, std::function<void(Args...)>&& h) noexcept : base_handler_t(id), _handler{std::move(h)} {}
        ~handler_t() noexcept override = default;
        handler_t(const handler_t&) = delete;
        handler_t(handler_t&&) noexcept = delete;
        handler_t& operator=(const handler_t&) = delete;
        handler_t& operator=(handler_t&&) = delete;
        
        void call(Args... args) const
        {
            _handler(std::forward<Args>(args)...);
        }

    private:
        std::function<void(Args...)> _handler;
    };

public:
    event_dispatcher() noexcept : _is_running{false}
    {
    }

    ~event_dispatcher() noexcept
    {
        stop();
        handler_id = 0;
    }

    event_dispatcher(const event_dispatcher&) = delete;
    event_dispatcher(event_dispatcher&&) noexcept = delete;
    event_dispatcher& operator=(const event_dispatcher&) = delete;
    event_dispatcher& operator=(event_dispatcher&&) = delete;

    template <typename... Args>
    constexpr auto get_event_handler_id(std::string&& event_name) const
    {
        if constexpr(sizeof...(Args) == 0)
        {
            return std::move(event_name);
        }
        else
        {
            auto args_hash = std::to_string((typeid(std::decay_t<Args>).hash_code() + ... ));
            return std::move(event_name + args_hash);
        }
    }

    template <typename... Args, typename HandlerT>
    auto add_handler(std::string event_name, HandlerT&& event_handler) -> unsigned long
    {
        using WrapperT = typename evds::details::function_traits<HandlerT>::wrapper_t;
        static_assert(std::is_same_v<void, std::invoke_result_t<HandlerT, Args...>>, "\nevent_handler must return void!");
        static_assert(std::is_same_v<std::function<void(std::decay_t<Args>...)>, WrapperT>, 
            "\nevent_handler arguments mismatch in add_handler()!"
            "\nplease match your handler arguments (input parameters) with your declaration (template specification)");

        const auto event_handler_id = get_event_handler_id<Args...>(std::move(event_name));

        std::unique_lock handlers_lock(_handlers_mutex);        
        _handlers[event_handler_id].emplace_back(std::make_unique<handler_t<Args...>>(++handler_id, std::move(event_handler)));
        return handler_id;
    }

    auto remove_handler(unsigned long handler_id) -> bool
    {
        std::unique_lock handlers_lock(_handlers_mutex);        

        for(auto&& event_handlers : _handlers)
        {
            if(std::erase_if(event_handlers.second, [handler_id](auto&& h){ return h->id == handler_id; }))
                return true;
        }

        return false;
    }

    template <typename... Args>
    auto emit(std::string event_name, Args... args) -> bool
    {
        const auto event_handler_id = get_event_handler_id<Args...>(std::move(event_name));

        std::unique_lock handlers_lock(_handlers_mutex);
        if (!_handlers.contains(event_handler_id))
            return false;

        std::vector<std::shared_ptr<base_handler_t>> event_handlers = _handlers.at(event_handler_id);
        handlers_lock.unlock();

        for (auto&& h : event_handlers)
        {
            const event_t event = [e = std::static_pointer_cast<handler_t<Args...>>(h), ... args = std::forward<Args>(args)]
            {
                return std::invoke(&handler_t<Args...>::call, e.get(), args...);
            };

            std::unique_lock events_lock(_events_mutex);
            _events_queue.emplace(event);
            events_lock.unlock();

            _dispatcher_cv.notify_one();
        }

        return true;
    }

    auto start(const unsigned int num_threads = 1) -> bool
    {
        {
            std::scoped_lock lock(_is_running_mutex);
            if(_is_running)
                return false;
        }

        auto dispatcher_threads_barrier_callback = [this]() noexcept
        {
            std::scoped_lock lock(_is_running_mutex);
            _is_running = true;
        };

        const auto thread_count = std::clamp(num_threads, 1u, std::thread::hardware_concurrency());
        _dispatcher_threads.reserve(thread_count);

        using barrier_t = std::barrier<decltype(dispatcher_threads_barrier_callback)>;
        auto dispatcher_threads_barrier = std::make_shared<barrier_t>(thread_count + 1, dispatcher_threads_barrier_callback);
        const auto dispatcher_thread = [this, &dispatcher_threads_barrier]()
        {
            dispatcher_threads_barrier->arrive_and_wait();
            run();
        };

        for (auto i = 0; i < thread_count; ++i)
            _dispatcher_threads.emplace_back(dispatcher_thread);

        dispatcher_threads_barrier->arrive_and_wait();
        return true;
    }

    auto stop() -> bool
    {
        {
            std::scoped_lock lock(_is_running_mutex);
            if(!_is_running)
                return false;
            
            _is_running = false;
        }

        _dispatcher_cv.notify_all();
        
        {
            std::scoped_lock handlers_lock(_handlers_mutex);
            _handlers.clear();
        }

        {
            std::scoped_lock events_lock(_events_mutex);
            _events_queue = {};
        }

        for (auto&& thread : _dispatcher_threads)
        {
            if (thread.joinable())
                thread.join();
        }
            
        return true;
    }

protected:
    void run()
    {
        while(_is_running)
        {
            std::unique_lock events_lock(_events_mutex);
            
            _dispatcher_cv.wait(events_lock, [this]{ return !_events_queue.empty() || !_is_running; });
            if (!_is_running)
                return;

            const event_t event = std::move(_events_queue.front());
            _events_queue.pop();
            events_lock.unlock();

            event();
        }
    }

private:
    std::atomic_bool _is_running;
    std::mutex _is_running_mutex;
    std::unordered_map<std::string, std::vector<std::shared_ptr<base_handler_t>>> _handlers;
    std::mutex _handlers_mutex;
    std::queue<event_t> _events_queue;
    std::mutex _events_mutex;
    std::vector<std::thread> _dispatcher_threads;
    std::condition_variable _dispatcher_cv;
    static unsigned long handler_id;
};

unsigned long event_dispatcher::handler_id = 0;

}
