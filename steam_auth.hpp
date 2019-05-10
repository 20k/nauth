#ifndef STEAM_AUTH_HPP_INCLUDED
#define STEAM_AUTH_HPP_INCLUDED

#include <optional>
#include <stdint.h>
#include <string>

struct app_description
{
    std::string secret_key_location;
    uint64_t appid = 0;
};

struct steam_auth_data
{
    uint64_t steam_id = 0;
    std::string user_data;
};

void set_app_description(const app_description& app);

std::optional<steam_auth_data> get_steam_auth(const std::string& hex_auth_data);

#endif // STEAM_AUTH_HPP_INCLUDED
