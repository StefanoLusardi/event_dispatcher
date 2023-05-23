#include <evds/event_dispatcher.hpp>
#include <iostream>
#include <sstream>

using namespace std::chrono_literals;

void thread_pool_N_threads(int num_threads)
{
    evds::event_dispatcher evds;
    evds.start(num_threads);

    std::string msg_start = "[Start] " + std::to_string(num_threads ) + " threads"  + "\n";
    std::cout << msg_start;
    
    evds.add_event_handler("event_id", [](const int arg)
    { 
        std::string out = "event_id(int): " + std::to_string(arg) + "\n";
        std::cout <<  out;
        std::this_thread::sleep_for(500ms);
        return arg + 1;
    });

    evds.add_event_handler("event_id", [](const std::string& arg)
    { 
        std::string out = "event_id(std::string): " + arg + "\n";
        std::cout << out;
        std::this_thread::sleep_for(1s);
        return arg + "-plus-one";
    });

    auto future1 = evds.push_event<int, int>("event_id", 42);
    auto future2 = evds.push_event<std::string, std::string>("event_id", std::string("fouty-two"));

    std::string f1_output = "future1(int): " + std::to_string(future1->get()) + "\n";
    std::cout << f1_output;
    
    std::string f2_output = "future2(std::string): " + future2->get()  + "\n";
    std::cout << f2_output;

    evds.stop();
    
    std::string msg_end = "[End] " + std::to_string(num_threads) + " threads" + "\n\n";
    std::cout << msg_end;
}

int main()
{
    thread_pool_N_threads(1); // single thread
    thread_pool_N_threads(4);  // 4 threads

    std::cout << "FINISH!" << std::endl;
    return 0;
}
