#pragma once

#include "meta.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace soci_orm
{
namespace json
{
template<typename T, typename Enable = void> struct cx
{
    static_assert(std::is_same<T, void>::value, "Class is not elligible for json conversion");
    static inline T deserialize(rapidjson::Value& v) { return T(); }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f) {}
};

template<typename T>
static inline T deserialize(const std::string& json)
{
    rapidjson::Document d;
    d.Parse(json.c_str());
    return cx<T>::deserialize(d);
}

template<typename T>
static inline std::string serialize(const T& field)
{
    rapidjson::Document d;
    cx<T>::serialize(d, d.GetAllocator(), field);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
    return buffer.GetString();
}

template<typename T>
static void deserialize_field(const std::string& name, const rapidjson::Value& src, T& dest)
{
    const auto m = src.FindMember(name);
    if (m != src.MemberEnd())
        dest = cx<T>::deserialize(m->value);
}

template<typename T>
static void serialize_field(const std::string& name, const T& src, rapidjson::Value& dest, rapidjson::Document::AllocatorType& a)
{
    rapidjson::Value v;
    cx<T>::serialize(v, a, src);
    dest.AddMember(rapidjson::Value(name, a), v, a);
}

template<typename T>
struct cx<T, typename std::enable_if_t<meta::is_collection_v<T> && !meta::is_basic_string_v<T>>>
{
    typedef typename T::value_type value_type;
    static inline T deserialize(const rapidjson::Value& v) {
        T collection;
        auto inserter = std::inserter(collection, collection.end());
        for (auto& v_ : v.GetArray()) {
            inserter = cx<value_type>::deserialize(v_);
        }
        return collection;
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetArray();
        for (const auto& val : f) {
            rapidjson::Value v;
            cx<value_type>::serialize(v, a, val);
            d.PushBack(v, a);
        }
    }
};

template<typename T>
struct cx<T, typename std::enable_if_t<meta::is_pair_v<T> && meta::is_basic_string_v<typename T::first_type>>>
{
    typedef std::decay_t<typename T::first_type> first_type;
    typedef std::decay_t<typename T::second_type> second_type;
    static inline T deserialize(const rapidjson::Value& v)
    {
        auto& val = *v.MemberBegin();
        return std::make_pair(cx<first_type>::deserialize(val.name), cx<second_type>::deserialize(val.value));
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        rapidjson::Value k;
        rapidjson::Value v;
        cx<first_type>::serialize(k, a, f.first);
        cx<second_type>::serialize(v, a, f.second);
        d.AddMember(k, v, a);
    }
};
template<typename T>
struct cx<T, typename std::enable_if_t<meta::is_pair_v<T> && !meta::is_basic_string_v<typename T::first_type>>>
{
    typedef std::decay_t<typename T::first_type> first_type;
    typedef std::decay_t<typename T::second_type> second_type;
    static inline T deserialize(const rapidjson::Value& v)
    {
        return std::make_pair(
            cx<first_type>::deserialize(v.FindMember("Key")->value),
             cx<second_type>::deserialize(v.FindMember("Value")->value));
    }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    {
        d.SetObject();
        rapidjson::Value v;
        cx<first_type>::serialize(v, a, f.first);
        d.AddMember("Key", v, a);
        cx<second_type>::serialize(v, a, f.second);
        d.AddMember("Value", v, a);
    }
};

template<typename T>
struct cx<T, typename std::enable_if_t<meta::is_basic_string_v<T>>>
{
    static inline T deserialize(const rapidjson::Value& v)
    { return v.GetString(); }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    { d = rapidjson::StringRef(f.c_str(), f.size()); }
};

template<>
struct cx<bool>
{
    typedef bool T;
    static inline T deserialize(const rapidjson::Value& v)
    { return v.GetBool(); }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    { d = f; }
};

template<typename T>
struct cx<T, typename std::enable_if_t<std::is_integral<T>::value>>
{
    static inline T deserialize(const rapidjson::Value& v)
    { return static_cast<T>(v.GetInt64()); }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    { d = f; }
};

template<typename T>
struct cx<T, typename std::enable_if_t<std::is_floating_point<T>::value>>
{
    static inline T deserialize(const rapidjson::Value& v)
    { return static_cast<T>(v.GetDouble()); }
    static inline void serialize(rapidjson::Value& d, rapidjson::Document::AllocatorType& a, const T& f)
    { d = f; }
};
} /* !namespace json */
} /* !namespace soci_orm */
