#include <evds/evds.hpp>


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


int main()
{
    using namespace std::chrono_literals;

    event_dispatcher bus;
    bus.start();

    bus.add_event_handler("foo", [](int arg) { std::cout << "foo " << arg << std::endl; });
    bus.add_event_handler("foo", []{ std::cout << "empty_2" << std::endl; });
    bus.add_event_handler("foo", []{ std::cout << "empty_1" << std::endl; });
    bus.add_event_handler("foo", [](std::string arg) { std::cout << "foo " << arg << std::endl; });
    bus.add_event_handler("foo", [](std::string& arg) { std::cout << "foo " << arg << std::endl; });
    bus.add_event_handler("foo", [](std::string&& arg) { std::cout << "foo " << arg << std::endl; });
    bus.add_event_handler("foo", [](const std::string& arg) { std::cout << "foo " << arg << std::endl; });
    bus.add_event_handler("foo", [](std::string arg) { std::cout << "foo " << arg << std::endl; });

    bus.push_event("foo");
    std::this_thread::sleep_for(1s);

    bus.push_event("bar");
    std::this_thread::sleep_for(1s);

    bus.push_event("foo", 1);
    std::this_thread::sleep_for(1s);

    bus.push_event<void, std::string>("foo", "string-2");
    std::this_thread::sleep_for(1s);

    bus.push_event("foo", Foo());
    std::this_thread::sleep_for(1s);

    bus.push_event("foo");
    bus.remove_event_handler("foo");
    bus.push_event("foo");
    std::this_thread::sleep_for(1s);
    
    auto func = [](int arg) { std::cout << "foo " << arg << std::endl; };
    bus.add_event_handler("foo", std::move(func));

    std::function<void(int, int)> func_free = foo;
    bus.add_event_handler("foo", std::move(func_free));
    
    std::function<void(int, int)> func_free2 = std::bind(foo, std::placeholders::_1, std::placeholders::_2);
    bus.add_event_handler("foo", std::move(func_free2));

    bus.add_event_handler("foo", &foo);

    bus.add_event_handler("foo_t", &foo_t<int>);
    bus.push_event("foo_t", 9999);

    Foo foo_obj;
    std::function<void(int, std::string)> foo_bind = std::bind(&Foo::bar, &foo_obj, std::placeholders::_1, std::placeholders::_2);
    bus.add_event_handler("foo_obj", std::move(foo_bind));

    std::function<void(std::string)> bar_bind = std::bind(&Foo::bar_t<std::string>, &foo_obj, std::placeholders::_1);
    bus.add_event_handler("foo_obj_bar_t", std::move(bar_bind));
    bus.push_event("foo_obj_bar_t", std::string("string-foo_obj_bar_t"));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "FINISH!" << std::endl;

    return 0;
}

// Message Bus:
// https://github.com/whichxjy/message-bus/blob/master/src/example.cpp

// Function Traits:
// https://functionalcpp.wordpress.com/2013/08/05/function-traits/