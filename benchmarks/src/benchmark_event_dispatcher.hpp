#pragma once

#include <benchmark/benchmark.h>
#include <evds/event_dispatcher.hpp>

namespace evds::benchmarks
{
class event_dispatcher_benchmark : public benchmark::Fixture
{
public:
    explicit event_dispatcher_benchmark() : e { std::make_unique<evds::event_dispatcher>() } { }
    void SetUp(benchmark::State& st) override { }
    void TearDown(benchmark::State& st) override { e->stop(); }

protected:
    std::unique_ptr<evds::event_dispatcher> e;
};

}