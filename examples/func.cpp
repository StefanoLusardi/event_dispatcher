#include <any>
#include <functional>
#include <iostream>
#include <vector>
#include <memory>
#include <map>
#include <string>

void foo(int i, int j)
{
    std::cout << "test - i: " << i << " j: " << j << std::endl;
}

template <typename T>
void foo_t(T arg)
{
    std::cout << "foo_t " << arg << std::endl;
}

class Foo
{
public:
    void bar(int i, std::string s)
    {
        std::cout << "test - i: " << i << " s: " << s << std::endl;
    }

    template <typename T>
    void bar_t(T arg)
    {
        std::cout << "bar_t " << arg << std::endl;
    }
};

struct event_base
{
    virtual ~event_base() = default;
};

template <typename... Args>
struct event : public event_base
{
    explicit event(std::function<void(Args...)>  f) : _f{f} {}
    ~event() override = default;
    void call(Args... args) {_f(args...);}

private:
    std::function<void(Args...)> _f;
};

class dispatcher
{
public:
    void add_event(const std::string& id, std::unique_ptr<event_base> e)
    {
        _events.emplace(id, std::move(e));
    }

    template<typename... Args>
    void call(const std::string& id, Args... args) 
    {
        auto e = _events.find(id);
        if(e == _events.end())
            return;

        event_base* evt_base = e->second.get();
        event<Args...>* evt = static_cast<event<Args...>*>(evt_base);
        evt->call(args...);
    }

private:
    std::map<std::string, std::unique_ptr<event_base>> _events;
};

int main()
{
    dispatcher d;
    auto f = [](){std::cout << "empty" << std::endl; return 0;};
    d.add_event("1", std::make_unique<event<>>(std::move(f)));
    d.add_event("2", std::make_unique<event<int>>([](int i){std::cout << "int: " << i << std::endl;}));
    d.add_event("3", std::make_unique<event<const char*>>([](std::string s){std::cout << "string: " << s << std::endl;}));
    d.add_event("4", std::make_unique<event<int, int>>(&foo));
    
    // d.add_event("1", std::move(f));

    d.call("1");
    d.call("1", 1);

    d.call("2", 2);
    d.call("2", Foo{});
    
    d.call("3", "three");
    d.call("3", std::string("three"));
    d.call("4", 7, 8);

    return 0;
}
