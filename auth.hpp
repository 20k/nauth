#ifndef AUTH_HPP_INCLUDED
#define AUTH_HPP_INCLUDED

#include <networking/serialisable.hpp>

namespace auth_type
{
    enum type : uint32_t
    {
        NONE = 0,
        STEAM = 1,
        CUSTOM = 2,
        COUNT
    };
}

template<typename user_data>
struct auth : serialisable
{
    user_data data;

    ///either a to_string'd steam_id or some other unique identification
    std::string user_id;

    auth_type::type type = auth_type::NONE;

    SERIALISE_SIGNATURE()
    {
        DO_SERIALISE(data);
        DO_SERIALISE(user_id);
        DO_SERIALISE(type);
    }
};

#endif // AUTH_HPP_INCLUDED
