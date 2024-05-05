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
    int value;
    std::vector<Inner> inner;
};

#define SOCI_ORM_MACRO              \
    SOCI_ORM_TABLE("INNER")         \
    SOCI_ORM_VALUE(value, "VALUE")  \

#define SOCI_ORM_CLASS Inner
#include "Database/soci_source_generate.hxx"

#define SOCI_ORM_MACRO              \
    SOCI_ORM_TABLE("DATA")          \
    SOCI_ORM_VALUE(value, "VALUE")  \
    SOCI_ORM_COLLECTION(inner)      \

#define SOCI_ORM_CLASS Data
#include "Database/soci_source_generate.hxx"

void test(soci::session& sql) {
    using ORM = soci_orm::ORM<Data>;
    std::cout << "Table: " << ORM::details::get_table() << "\n";
    std::cout << "FKs: [" << soci_orm::utils::merge_string(", ", ORM::details::get_fk()) << "]\n";
    std::cout << "PKs: [" << soci_orm::utils::merge_string(", ", ORM::details::get_pk()) << "]\n";
    std::cout << "Fields: [" << soci_orm::utils::merge_string(", ", ORM::details::get_fields()) << "]\n";
}

int main()
{
    soci::register_factory_sqlite3();
    remove("sqlite3.db");
    soci::session sql("sqlite3://db=:memory:");
    // sql.set_log_stream(&std::cout);
    std::cout << "Migration: " << soci_orm::json::serialize(soci_orm::ORM<Data>::migrate(sql)) << "\n";
    test(sql);
    return 0;
}
