#pragma once

#include <soci/soci.h>

#include "meta.h"

namespace soci_orm {
    template<typename T, typename Enable = void>
    struct Affinity
    {
        static_assert(std::is_same<T, void>::value, "Class has no conversion affinity");
        using orm_type = int;
        static constexpr soci::db_type db_type = soci::db_type::db_string;
        static inline void get(const soci::values& values, const std::string& name, T& field) { }
        static inline void set(soci::values& values, const std::string& name, const T& field) { }
        static inline orm_type to_bulk(const T& field) { return 0; }
    };

    // COLLECTIONS
    template<typename T>
    struct Affinity<T, typename std::enable_if<meta::is_collection_v<T> && !meta::is_basic_string_v<T>>::type>
    {
        using orm_type = std::string;
        static constexpr soci::db_type db_type = soci::db_type::db_string;
        static inline void get(const soci::values& values, const std::string& name, T& field)
        {
            std::string json = values.get<orm_type>(name, "");
            if (!json.empty())
                field = json::deserialize<T>(json);
        }
        static inline void set(soci::values& values, const std::string& name, const T& field)
        { values.set(name, json::serialize(field)); }
        static inline orm_type to_bulk(const T& field)
        { return field.size() == 0 ? std::string() : json::serialize(field); }
    };

    // STRING
    template<typename T>
    struct Affinity<T, typename std::enable_if<meta::is_basic_string_v<T>>::type>
    {
        using orm_type = std::string;
        static constexpr soci::db_type db_type = soci::db_type::db_string;
        static inline void get(const soci::values& values, const std::string& name, T& field)
        { field = (T)values.get<orm_type>(name); }
        static inline void set(soci::values& values, const std::string& name, const T& field)
        { values.set(name, field); }
        static inline orm_type to_bulk(const T& field)
        { return field; }
    };

    // ENUM - INTEGER - BOOL
    template<typename T>
    struct Affinity<T, typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type>
    {
        using orm_type = int64_t;
        static constexpr soci::db_type db_type = soci::db_type::db_int64;
        static inline void get(const soci::values& values, const std::string& name, T& field)
        { field = (T)values.get<orm_type>(name); }
        static inline void set(soci::values& values, const std::string& name, const T& field)
        { values.set(name, (int64_t)field); }
        static inline orm_type to_bulk(const T& field)
        { return (orm_type)field; }
    };

    // FLOAT
    template<typename T>
    struct Affinity<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
    {
        using orm_type = double;
        static constexpr soci::db_type db_type = soci::db_type::db_double;
        static inline void get(const soci::values& values, const std::string& name, T& field)
        { field = (T)values.get<orm_type>(name); }
        static inline void set(soci::values& values, const std::string& name, const T& field)
        { values.set(name, (double)field); }
        static inline orm_type to_bulk(const T& field)
        { return (orm_type)field; }
    };
}