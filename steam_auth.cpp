#include "steam_auth.hpp"
#include <stdint.h>
#include <steamworks_sdk_142/sdk/public/steam/steamencryptedappticket.h>
#include <vector>
#include <fstream>
#include <chrono>
#include <iostream>

int char_to_val(uint8_t c)
{
    if(c >= '0' && c <= '9')
        return c - '0';

    uint8_t low = tolower(c);

    if(low >= 'a' && low <= 'f')
        return (low - 'a') + 10;

    return 0;
}

std::string binary_to_hex(const std::string& in, bool swap_endianness)
{
    std::string ret;

    const char* LUT = "0123456789ABCDEF";

    for(auto& i : in)
    {
        int lower_bits = ((int)i) & 0xF;
        int upper_bits = (((int)i) >> 4) & 0xF;

        if(swap_endianness)
        {
            std::swap(lower_bits, upper_bits);
        }

        ret += std::string(1, LUT[lower_bits]) + std::string(1, LUT[upper_bits]);
    }

    return ret;
}

std::string hex_to_binary(const std::string& in, bool swap_endianness)
{
    std::string ret;

    int len = in.size();

    if((len % 2) != 0)
        return "Invalid Hex (non even size)";

    for(int i=0; i < len; i+=2)
    {
        int next = i + 1;

        if(next >= len)
            return "Invalid Hex (non even size)";

        char cchar = in[i];
        char nchar = next < len ? in[next] : '0';

        if(swap_endianness)
        {
            std::swap(cchar, nchar);
        }

        int lower = char_to_val(cchar) + (char_to_val(nchar) << 4);

        ret.push_back(lower);
    }

    return ret;
}

size_t get_time_ms()
{
    size_t milliseconds_since_epoch =
    std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();

    return milliseconds_since_epoch;
}

inline
std::string read_file_bin(const std::string& file)
{
    std::ifstream t(file, std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    if(!t.good())
        throw std::runtime_error("Could not open file " + file);

    return str;
}

app_description& get_app_description()
{
    static app_description location;

    return location;
}

void set_app_description(const app_description& app)
{
    get_app_description() = app;
}

std::optional<steam_auth_data> get_steam_auth(const std::string& hex_auth_data)
{
    std::vector<uint8_t> decrypted;
    decrypted.resize(1024);

    static std::string secret_key = hex_to_binary(read_file_bin(get_app_description().secret_key_location), true);

    uint32_t real_size = 1024;

    std::string binary_data = hex_to_binary(hex_auth_data, false);

    if(!SteamEncryptedAppTicket_BDecryptTicket((const uint8*)binary_data.c_str(), binary_data.size(), &decrypted[0], &real_size, (const uint8*)&secret_key[0], secret_key.size()))
    {
        printf("Failed to decrypt ticket\n");
        return std::nullopt;
    }

    decrypted.resize(real_size);

    if(decrypted.size() == 0)
    {
        printf("Decrypted size == 0\n");
        return std::nullopt;
    }

    //AppId_t found_appid = SteamEncryptedAppTicket_GetTicketAppID(&decrypted[0], decrypted.size());
    //std::cout << "appid is " << found_appid << std::endl;

    if(!SteamEncryptedAppTicket_BIsTicketForApp(&decrypted[0], decrypted.size(), get_app_description().appid))
    {
        printf("Ticket is for wrong appid\n");
        return std::nullopt;
    }

    RTime32 ticket_time = SteamEncryptedAppTicket_GetTicketIssueTime(&decrypted[0], decrypted.size());

    if(ticket_time == 0)
    {
        printf("Bad Ticket Time");
        return std::nullopt;
    }

    ///I have no idea what a good time here is
    size_t timeout = 120*1000;

    ///steam api returns time since unix epoch in seconds
    if((size_t)ticket_time*1000 + timeout < get_time_ms())
    {
        std::cout << "Ticket is too old " << ticket_time << " wall " << get_time_ms() << std::endl;
        return std::nullopt;
    }

    CSteamID steam_id;

    SteamEncryptedAppTicket_GetTicketSteamID(&decrypted[0], decrypted.size(), &steam_id);

    if(steam_id == k_steamIDNil)
    {
        printf("Invalid ticket (bad steam id)\n");
        return std::nullopt;
    }


    uint32 UserDataLength = 0;

    const uint8* user_data_ptr = SteamEncryptedAppTicket_GetUserVariableData(&decrypted[0], decrypted.size(), &UserDataLength);

    //std::cout << "steam id " << steam_id.ConvertToUint64() << std::endl;

    steam_auth_data ret;
    ret.steam_id = steam_id.ConvertToUint64();

    if(user_data_ptr != nullptr)
        ret.user_data = std::string((const char*)user_data_ptr, (uint32_t)UserDataLength);

    //std::cout << "Received user data of length " << UserDataLength << std::endl;

    return ret;
}
