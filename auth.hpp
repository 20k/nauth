#ifndef AUTH_HPP_INCLUDED
#define AUTH_HPP_INCLUDED

#include <networking/serialisable.hpp>
#include <map>
#include <mutex>
#include "steam_auth.hpp"

#define AUTH_DB_ID 0

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

    void handle_steam_auth(uint64_t id, const std::string& hex, const db_backend& db)
    {
        std::optional<steam_auth_data> steam_auth = get_steam_auth(hex);

        if(steam_auth)
        {
            std::cout << "auth with " << steam_auth.value().steam_id << std::endl;

            auth<T> user_auth;

            db_read_write tx(db, AUTH_DB_ID);

            if(user_auth.load(std::to_string(steam_auth.value().steam_id), tx))
            {
                std::cout << "Returning user\n";
            }
            else
            {
                std::cout << "New user\n";

                user_auth.key = std::to_string(steam_auth.value().steam_id);
                user_auth.user_id = std::to_string(steam_auth.value().steam_id);
                user_auth.type = auth_type::STEAM;

                user_auth.save(tx);
            }

            make_authenticated(id, user_auth);
        }
    }
};

#endif // AUTH_HPP_INCLUDED
