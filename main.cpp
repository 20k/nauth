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
        assert(test.load("20k", reader) == false);
    }
}

int main()
{
    auth_tests();

    return 0;
}
