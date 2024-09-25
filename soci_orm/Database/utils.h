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

    std::set<std::string> audit {};

    AuditRows& operator+=(const AuditRows& other) {
        inserted += other.inserted;
        updated += other.updated;
        merged += other.merged;
        ignored += other.ignored;
        loaded += other.loaded;
        return *this;
    }
};
typedef std::map<std::string, AuditRows> AuditTransaction;

struct AuditMigration
{
    enum class Action { CREATED, DELETED, CREATION_NOT_SUPPORTED };
    enum class Type { PK, FK, COL, TABLE };

    std::string name;
    Type type { Type::COL };
    Action action { Action::CREATED };

    friend bool operator<(const AuditMigration& l, const AuditMigration& r)
    {
        return l.name < r.name;
    }
};
typedef std::set<AuditMigration> AuditMigrationTable;
typedef std::map<std::string, AuditMigrationTable> AuditMigrationTables;

std::string merge_string(const std::string& separator, const std::vector<std::string>& values)
{
    std::string result;
    for (const auto& value : values)
        result.append(value).append(separator);
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
};

template<typename T>
struct smart_inserter<T, typename std::enable_if<!std::is_pointer<typename T::container_type::value_type>::value>::type>
{
    smart_inserter() = delete;
    template<typename U>
    static inline void insert(T& inserter, std::unique_ptr<U>& value) { inserter = std::move(*value); }
};

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
        json::deserialize_field("IGNORED", v, result.ignored);
        json::deserialize_field("LOADED", v, result.loaded);
        json::deserialize_field("AUDIT", v, result.audit);
        return result;
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        if (f.inserted > 0)     json::serialize_field("INSERTED", f.inserted, d, a);
        if (f.updated > 0)      json::serialize_field("UPDATED", f.updated, d, a);
        if (f.merged > 0)       json::serialize_field("MERGED", f.merged, d, a);
        if (f.ignored > 0)      json::serialize_field("IGNORED", f.ignored, d, a);
        if (f.loaded > 0)       json::serialize_field("LOADED", f.loaded, d, a);
        if (f.audit.size() > 0) json::serialize_field("AUDIT", f.audit, d, a);
    }
};

template<>
struct json::cx<utils::AuditMigration>
{
    typedef utils::AuditMigration T;
    static inline T deserialize(const rapidjson::Value& v)
    {
        return T {}; // Not used
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        std::string description;
        switch (f.type)
        {
            case T::Type::PK: description = "PK "; break;
            case T::Type::FK: description = "FK "; break;
            case T::Type::TABLE: description = "TABLE "; break;
            case T::Type::COL: description = "COLUMN "; break;
            default: break;
        }
        switch (f.action)
        {
            case T::Action::CREATED: description += "CREATED"; break;
            case T::Action::DELETED: description += "DELETED"; break;
            case T::Action::CREATION_NOT_SUPPORTED: description += "CREATION NOT SUPPORTED"; break;
            default: break;
        }
        d.AddMember(rapidjson::StringRef(f.name), rapidjson::Value(description, a), a);
    }
};

} /* !namespace soci_orm */