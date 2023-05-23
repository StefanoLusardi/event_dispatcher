#include <evds/event_dispatcher.hpp>
#include <iostream>
#include <sstream>

class application
{
public:
    static application* get_instance()
    { 
        static application instance;
        return &instance;
    }

    int run(int argc, char** argv)
    {
        e.start();

        e.stop();
        return 0;
    }
    
    template <typename RetType, typename... Args>
    void post(const std::string& event_name, Args... args)
    {
        e.push_event(event_name, std::forward<Args...>(args...));
    }

private:
    explicit application() = default;
    evds::event_dispatcher e;
};

class module_a
{
public:
    void exec()
    {
        auto app = application::get_instance();
        app
        app->post("module_a");

    }

private:
    std::thread _t;
};


int main(int argc, char** argv)
{
    application app;
    return app->run(argc, argv);
}