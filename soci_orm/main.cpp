// SociORM.cpp : Defines the entry point for the application.
//

#include <soci/soci.h>

#include <iostream>

#include "Database/soci_header.h"

namespace soci {
    extern "C" {
        void register_factory_sqlite3();
    }
}

struct Inner {
    std::string value;
};

struct Data {
    int integer;
    int value;
    std::vector<Inner> inner;
};

using Data_ = Data;

#define SOCI_ORM_MACRO              \
    SOCI_ORM_TABLE("INNER")         \
    SOCI_ORM_FK(Data_)               \
    SOCI_ORM_AUTOINDEX(autoindex, "AUTOINDEX") \
    SOCI_ORM_VALUE(value, "VALUE")  \

#define SOCI_ORM_CLASS Inner
#include "Database/soci_source_generate.hxx"

#define SOCI_ORM_MACRO                      \
    SOCI_ORM_TABLE("DATA")                  \
    SOCI_ORM_PK(integer, "PRIMARY_INTEGER") \
    SOCI_ORM_VALUE(value, "VALUE")          \
    SOCI_ORM_COLLECTION(inner)              \

#define SOCI_ORM_CLASS Data_
#include "Database/soci_source_generate.hxx"

template<typename T>
void test(soci_orm::Orm& orm) {
    using ORM = soci_orm::ORM<T>;
    std::cout << "\nTable: " << ORM::details::get_table() << "\n";
    std::cout << "FKs: [" << soci_orm::utils::merge_string(", ", ORM::details::get_fk()) << "]\n";
    std::cout << "PKs: [" << soci_orm::utils::merge_string(", ", ORM::details::get_pk()) << "]\n";
    std::cout << "Fields: [" << soci_orm::utils::merge_string(", ", ORM::details::get_fields()) << "]\n";
}

int main() {
    soci::register_factory_sqlite3();
    remove("sqlite3.db");
    soci_orm::Orm orm(soci::session("sqlite3://db=sqlite3.db"));
    orm.session.set_log_stream(&std::cout);
    std::cout << "Migration: " << soci_orm::json::serialize(soci_orm::ORM<Data>::migrate(orm)) << "\n";
    test<Data>(orm);
    test<Inner>(orm);
    std::vector<Data> datas = {
        Data { 1, 3, { { "A" }, { "B" } } },
        Data { 3, 5, { { "C" }, { "D" }, { "E" } } },
    };
    std::cout << soci_orm::json::serialize(soci_orm::ORM<Data>::save(orm, nullptr, datas)) << "\n";
    return 0;
}
