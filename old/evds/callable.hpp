#pragma once

#include <memory>

namespace evds::details
{
struct callable
{
    class callable_concept
    {
    public:
        virtual ~callable_concept() {}
        virtual void call() const = 0;

    protected:
        callable_concept() = default;
        callable_concept(const callable_concept&) = default;
        callable_concept(callable_concept&&) noexcept = default;
        callable_concept& operator=(const callable_concept&) = default;
        callable_concept& operator=(callable_concept&&) noexcept = default;
    };

    template <class T>
    class callable_model : public callable_concept
    {
    public:
        explicit callable_model(T&& callable) : _callable {std::forward<T>(callable)}
        {
            static_assert(std::is_invocable_v<decltype(callable)>);
        }

        void call() const override { _callable(); };

    private:
        mutable T _callable;
    };

public:
    template<class CallableT>
    explicit callable(CallableT&& callable) : _impl{ std::make_unique<callable_model<CallableT>>(std::move(callable)) } {}
    callable(callable&& other) noexcept : _impl{ std::move(other._impl) } {}
    ~callable() = default;
    void operator()() const { _impl->call(); };

protected:
    callable(const callable&) = delete;
    callable& operator=(const callable&) = delete;
    callable& operator=(callable&&) noexcept = delete;

private:
    std::unique_ptr<callable_concept> _impl;
};

}
