#include <iostream>
#include <networking/serialisable.hpp>
#include <networking/networking.hpp>
#include <ndb/db_storage.hpp>
#include "auth.hpp"
#include "steam_auth.hpp"
#include "steam_api.hpp"
#include <windows.h>

void auth_tests()
{
    ///this api is horrible
    set_db_location("./db");
    set_num_dbs(1);

    db_backend& db = get_db();

    {
        db_read reader(db, 0);

        auth<int> test;
        assert(test.load("dummy", reader) == false);
    }

    {
        {
            db_read_write tx(db, 0);

            auth<int> test;
            test.key = "i20k";
            test.user_id = "01234567";
            test.type = auth_type::CUSTOM;

            test.save(tx);
        }

        {
            db_read tx(db, 0);

            auth<int> test;

            assert(test.load("i20k", tx) && test.user_id == "01234567" && test.type == auth_type::CUSTOM);

            std::cout << "hello there " << test.user_id << std::endl;
        }

        {
            db_read_write tx(db, 0);

            tx.del("i20k");
        }
    }
}

void server()
{
    set_db_location("./db");
    set_num_dbs(1);

    std::string secret_key = "secret/akey.ect";
    uint64_t net_code_appid = 814820;

    set_app_description({secret_key, net_code_appid});

    connection conn;
    conn.host("127.0.0.1", 6750);

    auth_manager<int> auths;

    while(1)
    {
        while(conn.has_new_client())
        {
            printf("Nu client!\n");

            conn.pop_new_client();
        }

        ///so connection needs to understand whether or not someone is authed?
        ///or should auth go somewhere else?
        while(conn.has_read())
        {
            network_protocol proto;

            auto client_id = conn.reads_from(proto);

            if(proto.type == network_mode::STEAM_AUTH)
            {
                std::string hex = proto.data;

                auths.handle_steam_auth(client_id, hex, get_db());
            }

            if(!auths.authenticated(client_id))
            {
                printf("DENIED\n");

                conn.pop_read(client_id);
                continue;
            }

            if(proto.type == network_mode::DATA)
            {

            }

            conn.pop_read(client_id);
        }

        Sleep(1);
    }
}

void client()
{
    steamapi api;

    connection conn;
    conn.connect("127.0.0.1", 6750);

    api.request_auth_token("");

    while(!api.auth_success())
    {
        api.pump_callbacks();
    }

    std::vector<uint8_t> vec = api.get_encrypted_token();

    std::string str(vec.begin(), vec.end());

    network_protocol proto;
    proto.type = network_mode::STEAM_AUTH;
    proto.data = binary_to_hex(str, false);

    conn.writes_to(proto, -1);

    printf("Done!\n");

    while(1){}
}

int main()
{
    auth_tests();

    std::thread(server).detach();

    client();

    return 0;
}
