#include <evds/event_dispatcher.hpp>
#include <iostream>

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

    evds::event_dispatcher evds;
    evds.start();

    // Handler names overloads:
    // evds.add_event_handler("event_id_1", [] { std::cout << "event_id_1" << std::endl; });
    // evds.add_event_handler("event_id_2", [] { std::cout << "event_id_2" << std::endl; });

    // evds.push_event("event_id_1");
    // evds.push_event("event_id_2");

    // Parameter types overloads:    
    // evds.add_event_handler("foo", [](const char* arg) { std::cout << "void(const char*) - " << arg << std::endl; });
    evds.add_event_handler("foo", [](std::string arg) { std::cout << "void(std::string) - " << arg << std::endl; });
    evds.add_event_handler("foo", [](std::string&& arg) { std::cout << "void(std::string&&) -" << arg << std::endl; });
    // evds.add_event_handler("foo", [](const std::string& arg) { std::cout << "void(const std::string&) -  " << arg << std::endl; });

/*
    std::cout << "---" << std::endl;

    // evds.push_event("foo", "foo");
    // evds.push_event<void, std::string>("foo", "foo");
    // evds.push_event<void, std::string>("foo", std::string("foo"));
    // evds.push_event<void, const std::string&>("foo", std::string("foo"));
    evds.push_event<void, std::string&&>("foo", std::move(std::string("foo")));

    // Return types overloads:
    evds.add_event_handler("foo", [](int arg) -> void { std::cout << "void(int) - " << arg << std::endl; });
    evds.add_event_handler("foo", [](int arg) -> int { std::cout << "int(int) - " << arg << std::endl; return arg; });
    
    const int i = 123;
    evds.push_event<void, int>("foo", i);
    evds.push_event<int, int>("foo", i);

    // Custom objects:
    evds.push_event("foo", Foo{});
    
    // Free function:
    evds.add_event_handler("free_func", &foo);
    const int free_func_int = 12345;
    evds.push_event("free_func", free_func_int);
    
    // Free function template:
    evds.add_event_handler("free_func_t", &foo_t<int>);
    const int free_func_t_int = 12345;
    evds.push_event("free_func_t", free_func_t_int);

    // Member function with std::bind
    Foo foo_obj;
    std::function<void(int, std::string)> foo_bind = std::bind(&Foo::bar, &foo_obj, std::placeholders::_1, std::placeholders::_2);
    evds.add_event_handler("foo_obj", std::move(foo_bind));

    // Member function template with std::bind
    std::function<void(std::string)> bar_bind = std::bind(&Foo::bar_t<std::string>, &foo_obj, std::placeholders::_1);
    evds.add_event_handler("foo_obj_bar_t", std::move(bar_bind));
    evds.push_event("foo_obj_bar_t", std::string("string-foo_obj_bar_t"));

    //

    std::this_thread::sleep_for(1s);

    evds.push_event("bar");
    std::this_thread::sleep_for(1s);

    evds.push_event("foo", 1);
    std::this_thread::sleep_for(1s);

    evds.push_event<void, std::string>("foo", "string-2");
    std::this_thread::sleep_for(1s);

    evds.push_event("foo", Foo());
    std::this_thread::sleep_for(1s);

    evds.push_event("foo");
    evds.remove_event_handler("foo");
    evds.push_event("foo");
    std::this_thread::sleep_for(1s);

    auto func = [](int arg) { std::cout << "foo " << arg << std::endl; };
    evds.add_event_handler("foo", std::move(func));

    std::function<void(int, int)> func_free = foo;
    evds.add_event_handler("foo", std::move(func_free));

    std::function<void(int, int)> func_free2 = std::bind(foo, std::placeholders::_1, std::placeholders::_2);
    evds.add_event_handler("foo", std::move(func_free2));

    evds.add_event_handler("foo", &foo);

*/

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "FINISH!" << std::endl;

    return 0;
}

// Message Bus:
// https://github.com/whichxjy/message-evds/blob/master/src/example.cpp

// Function Traits:
// https://functionalcpp.wordpress.com/2013/08/05/function-traits/