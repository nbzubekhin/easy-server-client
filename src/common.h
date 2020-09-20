#pragma once

/// Description:
/// This file contains definitions that are extremely likely to be
/// needed in virtually all files.
#if __cplusplus < 201703L
    #error This project needs at least a C++17 compliant compiler
#endif

#include <unistd.h>

#include <algorithm>
#include <string>

/// Here is one possible implementation of System handle wrapper.
/// The idea was taken from
/// http://msdn.microsoft.com/en-gb/magazine/hh288076.aspx
template <typename Traits>
class unique_handle {
    using pointer = typename Traits::pointer;
    pointer m_value;

public:
    unique_handle(unique_handle const &) = delete;

    unique_handle &operator=(unique_handle const &) = delete;

    explicit unique_handle(pointer value = Traits::invalid()) noexcept
        : m_value{value} {
    }

    unique_handle(unique_handle &&other) noexcept : m_value{other.release()} {
    }

    unique_handle &operator=(unique_handle &&other) noexcept {
        if (this != &other) {
            this->reset(other.release());
        }
        return *this;
    }

    ~unique_handle() noexcept {
        Traits::close_handle(m_value);
    }

    explicit operator bool() const noexcept {
        return m_value != Traits::invalid();
    }

    pointer get() const noexcept {
        return m_value;
    }

    pointer release() noexcept {
        auto value = m_value;
        m_value    = Traits::invalid();
        return value;
    }

    bool reset(pointer value = Traits::invalid()) noexcept {
        if (m_value != value) {
            Traits::close_handle(m_value);
            m_value = value;
        }
        return static_cast<bool>(*this);
    }

    void swap(unique_handle<Traits> &other) noexcept {
        std::swap(m_value, other.m_value);
    }
};

struct unix_handle_traits {
    using pointer = int;
    static pointer invalid() noexcept {
        return -1;
    }
    static void close_handle(pointer value) noexcept {
        close(value);
    }
};

using socket_handle = unique_handle<unix_handle_traits>;

template <class Elem>
using tstring =
    std::basic_string<Elem, std::char_traits<Elem>, std::allocator<Elem>>;

template <class Elem>
using tstringstream =
    std::basic_stringstream<Elem, std::char_traits<Elem>, std::allocator<Elem>>;

template <typename Elem>
inline std::vector<tstring<Elem>> split(tstring<Elem> text,
                                        Elem const    delimiter) {
    auto sstr   = tstringstream<Elem>{text};
    auto tokens = std::vector<tstring<Elem>>{};
    auto token  = tstring<Elem>{};
    while (std::getline(sstr, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return tokens;
}
