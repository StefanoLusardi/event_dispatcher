#pragma once

#include <gtest/gtest.h>
#include <evds/event_dispatcher.hpp>

namespace evds::tests
{
class event_dispatcher_test : public ::testing::Test
{
protected:
    explicit event_dispatcher_test() : e { std::make_unique<evds::event_dispatcher>() } { }
    ~event_dispatcher_test() { }
    std::unique_ptr<evds::event_dispatcher> e;
};

}
