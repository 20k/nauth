#ifndef AUTH_HPP_INCLUDED
#define AUTH_HPP_INCLUDED

#include <networking/serialisable.hpp>
#include <map>
#include <mutex>

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
struct auth : serialisable, db_storable<auth<user_data>>
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

template<typename T>
struct auth_manager
{
    std::map<uint64_t, auth<T>> auths;
    std::mutex mut;

    [[nodiscard]]
    bool authenticated(uint64_t id)
    {
        std::lock_guard guard(mut);

        return auths.find(id) != auths.end();
    }

    void make_authenticated(uint64_t id, const auth<T>& in)
    {
        std::lock_guard guard(mut);

        auths[id] = in;
    }
};

#endif // AUTH_HPP_INCLUDED
