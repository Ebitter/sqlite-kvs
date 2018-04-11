#include "sqlite_kvs.h"

#include <iostream>

int main(){

    auto kvs = Sqlite_KVS<std::string, int64_t>("test.db", "kvs");

    kvs.upsert("hatguy", 4);

    //std::cout << kvs.get("Marc") << std::endl;
    std::cout << kvs.get("hatguy") << std::endl;

    return 0;
}
