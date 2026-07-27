#pragma once

#include <cstddef>
#include <cstdint>

template<uint32_t hash>
struct compile_time_helper
{
    static_assert(hash == -1);
    static constexpr bool print_hash = (hash == -1);
};

template<std::size_t N>
struct cstxpr_str
{
    char str[N]{};

    consteval cstxpr_str(const char (&literal)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
            str[i] = literal[i];
    }
};

template<std::size_t Capacity>
struct cstxpr_capped_str
{
    char str[Capacity]{};

    constexpr cstxpr_capped_str() = default;

    template<std::size_t N>
    consteval cstxpr_capped_str(const char (&literal)[N])
    {
        static_assert(N <= Capacity, "literal is longer than the capacity; raise it rather than truncating");

        for (std::size_t i = 0; i < N; ++i)
            str[i] = literal[i];
    }

    [[nodiscard]] constexpr const char* c_str() const
    {
        return str;
    }
};
