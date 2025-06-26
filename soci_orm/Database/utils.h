#pragma once

#include <map>
#include <set>
#include <string>

#include "private/json.h"

namespace soci_orm
{
namespace utils
{
enum class Action { INSERT, UPDATE, MERGE, IGNORE };
struct AuditRows
{
    int inserted {};
    int updated {};
    int merged {};
    int ignored {};
    int loaded {};
    int elapsed_ms {};

    std::set<std::string> audit {};

    AuditRows& operator+=(const AuditRows& other) {
        inserted += other.inserted;
        updated += other.updated;
        merged += other.merged;
        ignored += other.ignored;
        loaded += other.loaded;
        elapsed_ms += other.elapsed_ms;
        audit.insert(other.audit.begin(), other.audit.end());
        return *this;
    }
};
typedef std::map<std::string, AuditRows> AuditTransaction;

struct ColumnDefinition
{
    enum class Type { FK, PK, AUTOINDEX, COL };

    std::string name;
    Type type { Type::COL };
    soci::db_type db_type;
    size_t db_size { };
};
typedef std::vector<ColumnDefinition> ColumnsDefinition;
typedef std::map<std::string, ColumnsDefinition> TablesDefinition;
struct ColumnMigration {
    enum class Action { CREATED, DELETED, ALTERED, NO_ACTION };
    ColumnDefinition definition;
    Action action { Action::NO_ACTION };

    friend bool operator<(const ColumnMigration& l, const ColumnMigration& r)
    {
        return l.definition.name < r.definition.name;
    }

};
typedef std::set<ColumnMigration> ColumnsMigration;
typedef std::map<std::string, ColumnsMigration> TablesMigration;

std::string merge_string(const std::string& separator, const std::vector<std::string>& values)
{
    std::string result;
    for (const auto& value : values)
        result.append(value).append(separator);
    result.resize(std::max(0, ((int)result.size()) - ((int)separator.size())));
    return result;
}

template<typename T, typename U>
std::string merge_string(const std::string& separator, const T& values, const U& convertor)
{
    std::string result;
    for (const auto& value : values) {
        auto converted = convertor(value);
        if (!converted.empty())
            result.append(std::move(converted)).append(separator);
    }
    result.resize(std::max(0, ((int)result.size()) - ((int)separator.size())));
    return result;
}

template<typename T, typename Enable = void>
struct smart_inserter
{
    smart_inserter() = delete;
    static_assert(std::is_same<T, void>::value, "Smart inserter not handled");
    template<typename U>
    static inline void insert(T&, U*) {}
};

template<typename T>
struct smart_inserter<T, typename std::enable_if<std::is_pointer<typename T::container_type::value_type>::value>::type>
{
    smart_inserter() = delete;
    template<typename U>
    static inline void insert(T& inserter, std::unique_ptr<U>& value) { inserter = value.release(); }
    template<typename U>
    static inline void insert(T& inserter, std::unique_ptr<U>&& value) { inserter = value.release(); }
};

template<typename T>
struct smart_inserter<T, typename std::enable_if<meta::is_smartpointer_v<typename T::container_type::value_type>>::type>
{
    smart_inserter() = delete;
    template<typename U>
    static inline void insert(T& inserter, std::unique_ptr<U>& value) { inserter = std::move(value); }
    template<typename U>
    static inline void insert(T& inserter, std::unique_ptr<U>&& value) { inserter = std::move(value); }
};

template<typename T>
struct smart_inserter<T, typename std::enable_if<!std::is_pointer<typename T::container_type::value_type>::value && !meta::is_smartpointer_v<typename T::container_type::value_type>>::type>
{
    smart_inserter() = delete;
    template<typename U>
    static inline void insert(T& inserter, std::unique_ptr<U>& value) { inserter = std::move(*value); }
    template<typename U>
    static inline void insert(T& inserter, std::unique_ptr<U>&& value) { inserter = std::move(*value); }
};

template<typename T>
inline int compare(const T& a, const T& b) {
    static_assert(std::is_integral<T>::value, "Default compare requires integral values");
    return (int)a - b;
}
template<>
inline int compare(const std::string& a, const std::string& b) {
    return a.compare(b);
}

} /* !namespace utils */

template<>
struct json::cx<utils::AuditRows>
{
    typedef utils::AuditRows T;
    static inline T deserialize(const rapidjson::Value& v)
    {
        T result;
        json::deserialize_field("INSERTED", v, result.inserted);
        json::deserialize_field("UPDATED", v, result.updated);
        json::deserialize_field("MERGED", v, result.merged);
        json::deserialize_field("LOADED", v, result.loaded);
        json::deserialize_field("IGNORED", v, result.ignored);
        json::deserialize_field("ELAPSED_MS", v, result.elapsed_ms);
        json::deserialize_field("AUDIT", v, result.audit);
        return result;
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        if (f.inserted > 0)     json::serialize_field("INSERTED", f.inserted, d, a);
        if (f.updated > 0)      json::serialize_field("UPDATED", f.updated, d, a);
        if (f.merged > 0)       json::serialize_field("MERGED", f.merged, d, a);
        if (f.loaded > 0)       json::serialize_field("LOADED", f.loaded, d, a);
        if (f.ignored > 0)      json::serialize_field("IGNORED", f.ignored, d, a);
        if (f.elapsed_ms > 0)   json::serialize_field("IGNORED", f.elapsed_ms, d, a);
        if (f.audit.size() > 0) json::serialize_field("AUDIT", f.audit, d, a);
    }
};

template<>
struct json::cx<utils::ColumnMigration>
{
    typedef utils::ColumnMigration T;
    static inline T deserialize(const rapidjson::Value& v)
    {
        return T {}; // Not used
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        std::string description;
        switch (f.definition.type)
        {
            case utils::ColumnDefinition::Type::PK: description = "PK "; break;
            case utils::ColumnDefinition::Type::FK: description = "FK "; break;
            case utils::ColumnDefinition::Type::COL: description = "COLUMN "; break;
            default: break;
        }
        switch (f.action)
        {
            case T::Action::CREATED: description += "CREATED"; break;
            case T::Action::DELETED: description += "DELETED"; break;
            case T::Action::ALTERED: description += "ALTERED"; break;
            default: break;
        }
        d.AddMember(rapidjson::StringRef(f.definition.name), rapidjson::Value(description, a), a);
    }
};

} /* !namespace soci_orm */