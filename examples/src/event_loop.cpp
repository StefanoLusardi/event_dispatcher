#include <thread>
#include <chrono>
#include <iostream>
#include <evds/dispatcher.hpp>

using namespace std::chrono_literals;

int main() 
{
    evds::dispatcher d;

    d.subscribe("foo", [](int i){ std::cout << "foo called with" << i << std::endl;});

    // d.publish("foo", 1, "some_string");

    std::this_thread::sleep_for(5s);
    std::cout << "shutdown" << std::endl;
    std::cout << "finished" << std::endl;
    return 0;
}
