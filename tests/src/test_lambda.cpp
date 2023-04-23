#include "test_event_dispatcher.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <type_traits>

namespace evds::test
{
using evds_lambda = event_dispatcher_test;

// NOLINTNEXTLINE
TEST_F(evds_lambda, start_twice)
{
    EXPECT_TRUE(e->start());
    EXPECT_FALSE(e->start());
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, stop_twice)
{
    EXPECT_FALSE(e->stop());
    EXPECT_TRUE(e->start());
    
    EXPECT_TRUE(e->stop());
    EXPECT_FALSE(e->stop());
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, add_event_handler)
{
    EXPECT_TRUE(e->add_event_handler("", [](){}));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, remove_missing_event_handler)
{
    EXPECT_FALSE(e->remove_event_handler(""));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, add_event_handler_twice_same_name)
{
    EXPECT_TRUE(e->add_event_handler("", [](){}));
    EXPECT_FALSE(e->add_event_handler("", [](){}));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, remove_event_handler_twice_same_name)
{
    EXPECT_FALSE(e->remove_event_handler(""));
    EXPECT_FALSE(e->remove_event_handler(""));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, add_event_handler_twice_different_name)
{
    EXPECT_TRUE(e->add_event_handler("1", [](){}));
    EXPECT_TRUE(e->add_event_handler("2", [](){}));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, remove_event_handler_after_add)
{
    EXPECT_TRUE(e->add_event_handler("", [](){}));
    EXPECT_TRUE(e->remove_event_handler(""));
    EXPECT_FALSE(e->remove_event_handler(""));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, mix_add_remove_event_handlers)
{
    EXPECT_TRUE(e->add_event_handler("1", [](){}));
    EXPECT_TRUE(e->add_event_handler("2", [](){}));
    EXPECT_FALSE(e->add_event_handler("2", [](){})); // already added

    EXPECT_TRUE(e->remove_event_handler("1"));
    EXPECT_FALSE(e->remove_event_handler("1")); // already removed

    EXPECT_FALSE(e->remove_event_handler("3")); // never added
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, add_event_handler_after_start)
{
    e->start();
    EXPECT_TRUE(e->add_event_handler("", [](){}));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, remove_event_handler_after_start)
{
    e->start();
    EXPECT_TRUE(e->add_event_handler("", [](){}));
    EXPECT_TRUE(e->remove_event_handler(""));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, add_event_handler_after_stop)
{
    e->start();
    e->stop();
    EXPECT_TRUE(e->add_event_handler("", [](){}));
}


// NOLINTNEXTLINE
TEST_F(evds_lambda, remove_event_handler_after_stop)
{
    e->start();
    e->stop();
    EXPECT_TRUE(e->add_event_handler("", [](){}));
    EXPECT_TRUE(e->remove_event_handler(""));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, push_event_n_times_void_payload)
{
    std::mutex mtx;
    std::condition_variable cv;
    e->start();
    
    const auto event_name = "EVENT_NAME";
    int count = -1;
    e->add_event_handler(event_name, [&count, &cv]
    {
        count++;
        cv.notify_one();
    });

    for (int expected_count = 0; expected_count < 10; ++expected_count)
    {
        std::unique_lock lock(mtx);
        EXPECT_TRUE(e->push_event(event_name));
        cv.wait(lock, [&count, &expected_count]{ return count == expected_count; });
        EXPECT_EQ(count, expected_count);
    }
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, push_event_n_times_increasing_payload)
{
    std::mutex mtx;
    std::condition_variable cv;
    e->start();
    
    const auto event_name = "EVENT_NAME";
    int count = -1;
    e->add_event_handler(event_name, [&count, &cv](int expected_count_arg)
    {
        count++;
        EXPECT_EQ(count, expected_count_arg);
        cv.notify_one();
    });

    for (int expected_count = 0; expected_count < 10; ++expected_count)
    {
        std::unique_lock lock(mtx);
        EXPECT_TRUE(e->push_event(event_name, expected_count));
        cv.wait(lock, [&count, &expected_count]{ return count == expected_count; });
    }
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, push_event_n_times_same_payload)
{
    std::mutex mtx;
    std::condition_variable cv;
    e->start();
    
    const auto event_name = "EVENT_NAME";
    const int expected_value = 123456789;
    int count = -1;
    e->add_event_handler(event_name, [&expected_value, &count, &cv](int expected_value_arg)
    {
        count++;
        EXPECT_EQ(expected_value_arg, expected_value);
        cv.notify_one();
    });

    for (int expected_count = 0; expected_count < 10; ++expected_count)
    {
        std::unique_lock lock(mtx);
        EXPECT_TRUE(e->push_event(event_name, expected_value));
        cv.wait(lock, [&count, &expected_count]{ return count == expected_count; });
    }
}


// NOLINTNEXTLINE
TEST_F(evds_lambda, add_same_name_multiple_handlers)
{
    const auto event_name = "EVENT_NAME";
    EXPECT_TRUE(e->add_event_handler(event_name, [](int i){}));
    EXPECT_TRUE(e->add_event_handler(event_name, [](unsigned int i){}));
    EXPECT_TRUE(e->add_event_handler(event_name, [](unsigned long i){}));
    EXPECT_TRUE(e->add_event_handler(event_name, [](long long i){}));

    EXPECT_TRUE(e->add_event_handler(event_name, [](std::string s){}));
    EXPECT_TRUE(e->add_event_handler(event_name, [](std::string& s){}));
    EXPECT_TRUE(e->add_event_handler(event_name, [](std::string&& s){}));
    EXPECT_TRUE(e->add_event_handler(event_name, [](const std::string& s){}));
    
    EXPECT_FALSE(e->add_event_handler(event_name, [](const std::string s){})); // same as non-const, non-ref
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, push_multiple_handlers_same_event_name)
{
    std::mutex mtx;
    std::condition_variable cv;
    e->start();

    const auto event_name = "EVENT_NAME";

    auto int_payload = 0;
    EXPECT_TRUE(e->add_event_handler(event_name, [&cv, &int_payload](int i)
    {
        int_payload = i;
        cv.notify_one();
    }));

    std::string str_payload = "";
    EXPECT_TRUE(e->add_event_handler(event_name, [&cv, &str_payload](std::string s)
    {
        str_payload = s;
        cv.notify_one();
    }));

    std::unique_lock lock(mtx);

    int expected_value_int = 123456789;
    EXPECT_TRUE(e->push_event(event_name, expected_value_int));
    cv.wait(lock, [&expected_value_int, &int_payload]{ return expected_value_int == int_payload; });
    EXPECT_EQ(expected_value_int, int_payload);

    std::string expected_value_str = "some_random_string";
    EXPECT_TRUE(e->push_event(event_name, expected_value_str));
    cv.wait(lock, [&expected_value_str, &str_payload]{ return expected_value_str == str_payload; });
    EXPECT_EQ(expected_value_str, str_payload);
}


// NOLINTNEXTLINE
TEST_F(evds_lambda, push_event_consecutive_start_stop)
{
    std::mutex mtx;
    std::condition_variable cv;
    e->start();
    
    const auto event_name = "EVENT_NAME";
    int payload_value = 0;
    e->add_event_handler(event_name, [&payload_value, &cv](int expected_value_arg)
    {
        payload_value = expected_value_arg;
        cv.notify_one();
    });

    std::unique_lock lock(mtx);

    int expected_value1 = 111;
    EXPECT_TRUE(e->push_event(event_name, expected_value1));
    cv.wait(lock, [&payload_value, &expected_value1]{ return expected_value1 == payload_value; });
    EXPECT_EQ(expected_value1, payload_value);

    lock.unlock();
    e->stop();
    lock.lock();

    int expected_value2 = 222;
    EXPECT_FALSE(e->push_event(event_name, expected_value2));
    cv.wait(lock, [&payload_value, &expected_value1]{ return payload_value == expected_value1; });
    EXPECT_EQ(expected_value1, payload_value); // still match expected_value1, not expected_value2

    lock.unlock();
    e->start();
    lock.lock();

    int expected_value3 = 333;
    EXPECT_FALSE(e->push_event(event_name, expected_value3)); // handler not registered yet.

    // Add again the previous event handler since stop() clears all event handlers.
    e->add_event_handler(event_name, [&payload_value, &cv](int expected_value_arg)
    {
        payload_value = expected_value_arg;
        cv.notify_one();
    });

    EXPECT_TRUE(e->push_event(event_name, expected_value3));
    cv.wait(lock, [&payload_value, &expected_value3]{ return payload_value == expected_value3; });
    EXPECT_EQ(expected_value3, payload_value); // now match expected_value3
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, push_missing_event)
{
    e->start();
    const auto event_name = "EVENT_NAME";
    EXPECT_FALSE(e->push_event(""));
    EXPECT_FALSE(e->push_event("", 1, 2, 3));
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, push_event_with_different_overloads)
{
    e->start();
    const auto event_name = "EVENT_NAME";

    EXPECT_TRUE(e->add_event_handler(event_name, [](float f1, float f2, float f3){}));
    EXPECT_TRUE((e->push_event<void, float, float, float>(event_name, 1.f, 2.f, 3.f)));
    EXPECT_FALSE((e->push_event<void, int, int, int>(event_name, 1, 2, 3))); // no match for int,int,int

    EXPECT_TRUE(e->add_event_handler(event_name, [](int i1, int i2, int i3){}));
    EXPECT_TRUE((e->push_event<void, int, int, int>(event_name, 1, 2, 3))); // now can match previous handler
}

// NOLINTNEXTLINE
TEST_F(evds_lambda, remove_event_with_arguments)
{
    const auto event_name = "EVENT_NAME";
    EXPECT_TRUE(e->add_event_handler(event_name, [](int i, std::string s){}));
    EXPECT_FALSE((e->remove_event_handler<void, int>(event_name))); // no match void(int)
    EXPECT_TRUE((e->remove_event_handler<void, int, std::string>(event_name))); // match: void(int, string)
    EXPECT_FALSE((e->remove_event_handler<void, int, std::string>(event_name))); // already removed (previous line)
}

}