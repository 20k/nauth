#include <iostream>
#include <networking/serialisable.hpp>
#include <ndb/db_storage.hpp>
#include "auth.hpp"

void auth_tests()
{
    set_db_location("./test");
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

int main()
{
    auth_tests();

    return 0;
}
