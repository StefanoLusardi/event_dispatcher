#pragma once

namespace evds
{
class event
{
public:
    virtual ~event() = default;

    // event(event&) = delete;
    // event(const event&) = delete;
    // event(event&&) noexcept = delete;
    // event& operator=(const event&) = delete;
    // event& operator=(event&&) = delete;
};

}
