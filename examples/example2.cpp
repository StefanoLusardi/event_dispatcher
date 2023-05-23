#include <evds/event_dispatcher.hpp>
#include <iostream>

using namespace std::chrono_literals;

int main()
{
    evds::event_dispatcher e;
    e.start(4);

    /*
    Does not compile:
    e.add_handler<std::string>("event!", [](float arg){std::cout << arg << std::endl; });

    static_assert:
        event_handler arguments mismatch in add_handler()!
        please match your handler arguments (input parameters) with your declaration (template specification)
    */

    /*
    Does not compile:
    e.add_handler<std::string>("event!", [](std::string arg){std::cout << arg << std::endl; return 42; });

    static_assert:
        event_handler must return void!
    */

    auto e1 = e.add_handler<std::string>("Event_A", [](const std::string& arg){ std::this_thread::sleep_for(1s); std::cout << arg + "1\n"; });
    auto e2 = e.add_handler<std::string>("Event_A", [](const std::string& arg){ std::this_thread::sleep_for(1s); std::cout << arg + "2\n"; });
    auto e3 = e.add_handler<std::string>("Event_A", [](const std::string& arg){ std::this_thread::sleep_for(1s); std::cout << arg + "3\n"; });
    e.emit("Event_A", std::string("A"));

    bool r1 = e.remove_handler(e1); // true
    bool r2 = e.remove_handler(e3); // true
    bool r3 = e.remove_handler(0);  // false (id does not exists)
    bool r4 = e.remove_handler(e3); // false (already removed)
    e.emit("Event_A", std::string("A"));

    // e.add_handler<float>("e2", [](float arg){std::cout << arg << std::endl; });
    // e.add_handler<std::string>("e2", [](const std::string& arg){std::cout << arg << std::endl; });

    // std::this_thread::sleep_for(1s);
    std::cout << "FINISH!" << std::endl;

    return 0;
}
