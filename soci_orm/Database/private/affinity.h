#pragma once

#include <soci/soci.h>

#include "meta.h"

namespace soci_orm {
    template<typename T, typename Enable = void>
    struct Affinity
    {
        static_assert(std::is_same<T, void>::value, "Class has no conversion affinity");
        using orm_type = std::nullptr_t;
        static constexpr soci::db_type db_type = soci::db_type::db_string;
        static constexpr int db_size = 0;

        static inline void from_bulk(const orm_type& value, T& field) { }
        static inline orm_type to_bulk(const T& field) { return 0; }
    };

    // COLLECTIONS
    template<typename T>
    struct Affinity<T, typename std::enable_if<meta::is_collection_v<T> && !meta::is_basic_string_v<T>>::type>
    {
        using orm_type = std::string;
        static constexpr soci::db_type type = soci::db_type::db_string;
        static constexpr int db_size = 0;

        static inline void from_bulk(const orm_type& value, T& field)
        { if (!value.empty()) field = json::deserialize<T>(value); }
        static inline orm_type to_bulk(const T& field)
        { return field.size() == 0 ? std::string() : json::serialize(field); }
    };

    // STRING
    template<typename T>
    struct Affinity<T, typename std::enable_if<meta::is_basic_string_v<T>>::type>
    {
        using orm_type = std::string;
        static constexpr soci::db_type db_type = soci::db_type::db_string;
        static constexpr int db_size = 255;

        static inline void from_bulk(const orm_type& value, T& field)
        { field = value; }
        static inline orm_type to_bulk(const T& field)
        { return field; }
    };

    // ENUM - INTEGER - BOOL
    template<typename T>
    struct Affinity<T, typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type>
    {
        using orm_type = int64_t;
        static constexpr soci::db_type db_type = soci::db_type::db_int64;
        static constexpr int db_size = 0;
        
        static inline void from_bulk(const orm_type& value, T& field)
        { field = (T)value; }
        static inline orm_type to_bulk(const T& field)
        { return (orm_type)field; }
    };

    // FLOAT
    template<typename T>
    struct Affinity<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
    {
        using orm_type = double;
        static constexpr soci::db_type db_type = soci::db_type::db_double;
        static constexpr int db_size = 0;

        static inline void from_bulk(const orm_type& value, T& field)
        { field = (T)value; }
        static inline orm_type to_bulk(const T& field)
        { return (orm_type)field; }
    };
}