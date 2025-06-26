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
    int integer { };
    int value { };
    std::vector<Inner> inner;
};

#define SOCI_ORM_MACRO              \
    SOCI_ORM_TABLE("INNER")         \
    SOCI_ORM_FK(Data)               \
    SOCI_ORM_AUTOINDEX(autoindex, "AUTOINDEX") \
    SOCI_ORM_VALUE(value, "VALUE")  \

#define SOCI_ORM_CLASS Inner
#include "Database/soci_source_generate.hxx"

#define SOCI_ORM_MACRO                      \
    SOCI_ORM_TABLE("DATA")                  \
    SOCI_ORM_PK(integer, "PRIMARY_INTEGER") \
    SOCI_ORM_VALUE(value, "VALUE")          \
    SOCI_ORM_COLLECTION(inner)              \

#define SOCI_ORM_CLASS Data
#include "Database/soci_source_generate.hxx"

template<>
struct soci_orm::json::cx<Inner>
{
    typedef Inner T;
    static inline T deserialize(const rapidjson::Value& v)
    {
        T result;
        json::deserialize_field("VALUE", v, result.value);
        return result;
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        json::serialize_field("VALUE", f.value, d, a);
    }
};

template<>
struct soci_orm::json::cx<Data>
{
    typedef Data T;
    static inline T deserialize(const rapidjson::Value& v)
    {
        T result;
        json::deserialize_field("INTEGER", v, result.integer);
        json::deserialize_field("VALUE", v, result.value);
        json::deserialize_field("INNER", v, result.inner);
        return result;
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        json::serialize_field("INTEGER", f.integer, d, a);
        json::serialize_field("VALUE", f.value, d, a);
        json::serialize_field("INNER", f.inner, d, a);
    }
};


template<typename T>
void test(soci_orm::Orm& orm) {
    using ORM = soci_orm::ORM<T>;
    using CD = soci_orm::utils::ColumnDefinition;
    std::cout << "\nTable: " << ORM::details::get_table() << "\n";
    std::cout << "FKs: [" << soci_orm::utils::merge_string(", ", ORM::details::get_columns_definition(), 
        [](const CD& v){ return v.type == CD::Type::FK ? v.name : ""; }) << "]\n";
    std::cout << "PKs: [" << soci_orm::utils::merge_string(", ", ORM::details::get_columns_definition(),
        [](const CD& v){ return (v.type == CD::Type::PK || v.type == CD::Type::AUTOINDEX) ? v.name : ""; }) << "]\n";
    std::cout << "Fields: [" << soci_orm::utils::merge_string(", ", ORM::details::get_columns_definition(),
        [](const CD& v){ return v.type == CD::Type::COL ? v.name : ""; }) << "]\n";
}

#include "catch_amalgamated.hpp"

TEST_CASE("Workflow", "[workflow]")
{
    soci::register_factory_sqlite3();
    soci_orm::Orm orm(soci::session("sqlite3://db=:memory:"));
    orm.session.set_log_stream(&std::cout);
    auto migration = soci_orm::ORM<Data>::migrate(orm);

    std::cout << "Migration: " << soci_orm::json::serialize(migration) << "\n";
    test<Data>(orm);
    test<Inner>(orm);
    std::vector<Data> datas = {
        Data { 1, 3, { { "A" }, { "B" } } },
        Data { 3, 5, { { "C" }, { "D" }, { "E" } } },
    };
    auto insertions = soci_orm::ORM<Data>::save(orm, nullptr, datas);
    std::cout << "Saving: " << soci_orm::json::serialize(insertions) << "\n";

    std::vector<Data> loaded_data;
    auto load = soci_orm::ORM<Data>::load(orm, "", loaded_data);
    std::cout << "Loading: " << soci_orm::json::serialize(load) << "\n";
    CHECK(soci_orm::json::serialize(datas) == soci_orm::json::serialize(loaded_data));
}
