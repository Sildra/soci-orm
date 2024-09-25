template<>
struct soci_orm::Loader<SOCI_ORM_CLASS>::ForeignKey::Impl
{
    static constexpr bool can_be_less = false
#define SOCI_ORM_FK(CLASS) || true
#include "soci_xmacro.h"
        ;
    
#define SOCI_ORM_FK(CLASS)  \
    Loader<CLASS>::PrimaryKey foreign_key_ ## CLASS;
#include "soci_xmacro.h"

    Impl() = default;

    Impl(const Impl& other)
    {
#define SOCI_ORM_FK(CLASS)  \
        foreign_key_ ## CLASS.copy_from(other.foreign_key_ ## CLASS);
#include "soci_xmacro.h"
    }

    static void from_base(const soci::values& values, soci::indicator ind, Impl& t)
    {
#define SOCI_ORM_FK(CLASS)  \
        t.foreign_key_ ## CLASS.from_base(values, ind);
#include "soci_xmacro.h"
    }

    static void to_base(const Impl& t, soci::values& values, soci::indicator& ind)
    {
#define SOCI_ORM_FK(CLASS)  \
        t.foreign_key_ ## CLASS.to_base(values, ind);
#include "soci_xmacro.h"
    }

    bool operator<(const Impl& other) const
    {
        constexpr bool tie_item = true;
        return std::tie(tie_item
#define SOCI_ORM_FK(CLASS)  \
                , foreign_key_ ## CLASS
#include "soci_xmacro.h"
        ) < std::tie(tie_item
#define SOCI_ORM_FK(CLASS)  \
                , other.foreign_key_ ## CLASS
#include "soci_xmacro.h"
        );
    }
};

template<>
struct soci_orm::Loader<SOCI_ORM_CLASS>::PrimaryKey::Impl {
    static constexpr bool can_be_less = false
#define SOCI_ORM_PK(FIELD, NAME) || true
#include "soci_xmacro.h"
        ;

#define SOCI_ORM_PK(FIELD, NAME) \
    decltype(SOCI_ORM_ACCESSOR::FIELD) primary_key_ ## FIELD;
#include "soci_xmacro.h"

    Impl() {}
    Impl(const SOCI_ORM_CLASS& value) {
#define SOCI_ORM_PK(FIELD, NAME) \
        primary_key_ ## FIELD = SOCI_ORM_ACCESSOR::get_ ## FIELD(value);
#include "soci_xmacro.h"
    }
    
    void copy_from(const Impl& other)
    {
#define SOCI_ORM_PK(FIELD, NAME)  \
        primary_key_ ## FIELD = other.primary_key_ ## FIELD;
#include "soci_xmacro.h"
    }

    void from_base(const soci::values& values, soci::indicator ind)
    {
#define SOCI_ORM_PK(FIELD, NAME)  \
        soci_orm::Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::get(values, NAME, primary_key_ ## FIELD);
#include "soci_xmacro.h"
    }

    void to_base(soci::values& values, soci::indicator& ind) const
    {
#define SOCI_ORM_PK(FIELD, NAME)  \
        soci_orm::Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::set(values, NAME, primary_key_ ## FIELD);
#include "soci_xmacro.h"
    }

    bool operator<(const Impl& other) const
    {
        constexpr bool tie_item = true;
        return std::tie(tie_item
#define SOCI_ORM_PK(FIELD, NAME)  \
                , primary_key_ ## FIELD
#include "soci_xmacro.h"
        ) < std::tie(tie_item
#define SOCI_ORM_PK(FIELD, NAME)  \
                , other.primary_key_ ## FIELD
#include "soci_xmacro.h"
        );
    }
};

template<>
struct soci_orm::Loader<SOCI_ORM_CLASS>::Mapper {
    Loader<SOCI_ORM_CLASS>::ForeignKey::Impl foreign_key;
    SOCI_ORM_CLASS value;
};

namespace soci {
template<>
struct type_conversion<::soci_orm::Loader<SOCI_ORM_CLASS>::ForeignKey::Impl> {
    typedef soci::values base_type;
    using Type = ::soci_orm::Loader<SOCI_ORM_CLASS>::ForeignKey::Impl;
    static void from_base(const soci::values& v, soci::indicator ind, Type& t) {
        t.from_base(v, ind, t);
    }
    static void to_base(const Type& t, soci::values& v, soci::indicator ind) {
        t.to_base(t, v, ind);
    }
};
template<>
struct type_conversion<::soci_orm::Loader<SOCI_ORM_CLASS>::Mapper> {
    typedef soci::values base_type;
    using Type = ::soci_orm::Loader<SOCI_ORM_CLASS>::Mapper;
    static void from_base(const soci::values& v, soci::indicator ind, Type& t) {
        //soci_orm::ORM<SOCI_ORM_CLASS>::details::from_base(v, ind, t.value);
        decltype(t.foreign_key)::from_base(v, ind, t.foreign_key);
    }
    static void to_base(const Type& t, soci::values& v, soci::indicator ind) {
        //soci_orm::ORM<SOCI_ORM_CLASS>::details::to_base(t.value, v, ind);
        decltype(t.foreign_key)::to_base(t.foreign_key, v, ind);
    }
};
} /* !namespace soci */

template<>
struct soci_orm::Loader<SOCI_ORM_CLASS>::Impl {
    std::map<Loader<SOCI_ORM_CLASS>::ForeignKey,
        std::map<Loader<SOCI_ORM_CLASS>::PrimaryKey,
            std::unique_ptr<SOCI_ORM_CLASS>>> data;

    mutable decltype(data.begin()) foreign_key_it;
    mutable decltype(foreign_key_it->second.begin()) primary_key_it;
    mutable utils::AuditTransaction transaction_result;

    using PlainData = std::map<std::reference_wrapper<const PrimaryKey>, SOCI_ORM_CLASS*, std::less<const PrimaryKey&>>;

    Impl(Orm& orm, std::string filter) {
        Mapper mapper;
        {
            soci::statement stmt = ORM<SOCI_ORM_CLASS>::details::prepare_select(orm.session, filter
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
                + " order by " + utils::merge_string(", ", ORM<SOCI_ORM_CLASS>::details::get_pk())
#include "soci_xmacro.h"
            );
            stmt.exchange(soci::into(mapper.foreign_key));
            stmt.define_and_bind();
            stmt.execute();
            while (stmt.fetch()) {
                data[ForeignKey(mapper.foreign_key)][PrimaryKey(mapper.value)] = std::make_unique<SOCI_ORM_CLASS>(std::move(mapper.value));
            }
        }
        if (data.size() == 0) {
            data[ForeignKey(mapper.foreign_key)];
            foreign_key_it = data.begin();
            primary_key_it = foreign_key_it->second.begin();
            return;
        }
        foreign_key_it = data.begin();
        primary_key_it = foreign_key_it->second.begin();

        // Create a plain container for the inner collections
        PlainData plain_data;
        if (PrimaryKey::Impl::can_be_less) {
            for (const auto& fks : data) {
                for (const auto& pks : fks.second) {
                    plain_data.insert(std::make_pair(std::cref(pks.first), pks.second.get()));
                }
            }
        }

        if (!filter.empty()) {
            auto where = utils::merge_string(", ", ORM<SOCI_ORM_CLASS>::details::get_pk());
            filter = " where (" + where + ") in "
                "(select " + where + " from " + ORM<SOCI_ORM_CLASS>::details::get_table() + filter + ")";
        }

#define SOCI_ORM_COLLECTION(FIELD)                                                          \
        {                                                                                   \
            using ValType = std::remove_pointer_t<decltype(SOCI_ORM_ACCESSOR::FIELD)::value_type>;  \
            process_collection<ValType>(orm, filter, plain_data, SOCI_ORM_ACCESSOR::mutable_get_ ## FIELD); \
        }
#include "soci_xmacro.h"
    }

    template<typename C, typename F, typename Enable = void>
    void process_collection(Orm& orm, const std::string& filter, const PlainData& plain_data, F callable)
    {
        static_assert(std::is_same<C, C>::value, "");
    }

    template<typename C, typename F, typename std::enable_if<!std::is_same<C, SOCI_ORM_CLASS>::value>::type>
    void process_collection(Orm& orm, const std::string& filter, const PlainData& plain_data, F callable)
    {
        Loader<C> loader(orm, filter);
        std::string issue;
        while (const typename Loader<C>::ForeignKey* fk = loader.peek_fk()) {
           
            PrimaryKey* pk = nullptr;
            fk->template fetch<SOCI_ORM_CLASS>(pk);
            if (pk == nullptr) {
                issue = "UNBOUND FOREIGN_KEY";
                break;
            }
            auto plain_data_item = plain_data.find(*pk);
            if (plain_data_item == plain_data.end()) {
                if (auto item = loader.fetch_pk())
                    issue = "UNMAPPED FOREIGN_KEY";
                loader.fetch_fk();
                continue;
            }
            auto& coll = callable(*plain_data_item->second);
            auto inserter = std::inserter(coll, coll.end());
            while (auto item = loader.fetch_pk()) {
                //utils::smart_inserter<decltype(inserter)>::insert(inserter, item);
            }
            loader.fetch_fk();
        }
        for (const auto& child : loader.get_transaction_result())
            transaction_result[child.first] += child.second;
        if (!issue.empty())
            transaction_result[ORM<C>::details::get_table()].audit.insert(issue);
    }

    std::unique_ptr<SOCI_ORM_CLASS> fetch()
    {
        while (true) {
            auto ret = fetch_pk();
            if (ret)
                return ret;
            if (!fetch_fk())
                return nullptr;
        }
    }

    std::unique_ptr<SOCI_ORM_CLASS> fetch_pk()
    {
        if (primary_key_it == foreign_key_it->second.end())
            return nullptr;
        auto ret = std::move(primary_key_it->second);
        ++primary_key_it;
        return ret;
    }

    const ForeignKey* fetch_fk()
    {
        if (foreign_key_it == data.end())
            return nullptr;
        ++foreign_key_it;
        if (foreign_key_it == data.end())
            return nullptr;
        primary_key_it = foreign_key_it->second.begin();
        return &foreign_key_it->first;
    }

    const ForeignKey* peek_fk() {
        if (foreign_key_it == data.end())
            return nullptr;
        return &foreign_key_it->first;
    }
};

namespace soci_orm {
template<> Loader<SOCI_ORM_CLASS>::ForeignKey::ForeignKey()
    : pimpl(std::make_unique<Impl>()) {}
template<> Loader<SOCI_ORM_CLASS>::ForeignKey::ForeignKey(const Impl& value)
    : pimpl(std::make_unique<Impl>(value)) {}
template<> Loader<SOCI_ORM_CLASS>::ForeignKey::ForeignKey(ForeignKey&& other) { pimpl.swap(other.pimpl); }
template<> Loader<SOCI_ORM_CLASS>::ForeignKey& Loader<SOCI_ORM_CLASS>::ForeignKey::operator=(ForeignKey&& other) { pimpl.swap(other.pimpl); return *this; }
template<> Loader<SOCI_ORM_CLASS>::ForeignKey::~ForeignKey() {}
template<> bool Loader<SOCI_ORM_CLASS>::ForeignKey::operator<(const ForeignKey& other) const { return *pimpl < *other.pimpl; }

#define SOCI_ORM_FK(CLASS)                                                                          \
template<>                                                                                          \
template<>                                                                                          \
void Loader<SOCI_ORM_CLASS>::ForeignKey::fetch<CLASS>(Loader<CLASS>::PrimaryKey*& value) const {    \
    value = &pimpl->foreign_key_ ## CLASS;                                                          \
}
#include "soci_xmacro.h"

template<> Loader<SOCI_ORM_CLASS>::PrimaryKey::PrimaryKey()
    : pimpl(std::make_unique<Impl>()) {}
template<> Loader<SOCI_ORM_CLASS>::PrimaryKey::PrimaryKey(const SOCI_ORM_CLASS& value)
    : pimpl(std::make_unique<Impl>(value)) {}
template<> Loader<SOCI_ORM_CLASS>::PrimaryKey::PrimaryKey(PrimaryKey&& other) { pimpl.swap(other.pimpl); }
template<> Loader<SOCI_ORM_CLASS>::PrimaryKey& Loader<SOCI_ORM_CLASS>::PrimaryKey::operator=(PrimaryKey&& other) { pimpl.swap(other.pimpl); return *this; }
template<> Loader<SOCI_ORM_CLASS>::PrimaryKey::~PrimaryKey() {}
template<> bool Loader<SOCI_ORM_CLASS>::PrimaryKey::operator<(const PrimaryKey& other) const { return *pimpl < *other.pimpl; }
template<> void Loader<SOCI_ORM_CLASS>::PrimaryKey::copy_from(const PrimaryKey& other) { pimpl->copy_from(*other.pimpl); }
template<> void Loader<SOCI_ORM_CLASS>::PrimaryKey::from_base(const soci::values& values, soci::indicator ind) { pimpl->from_base(values, ind); }
template<> void Loader<SOCI_ORM_CLASS>::PrimaryKey::to_base(soci::values& values, soci::indicator ind) const { pimpl->to_base(values, ind); }

template<> Loader<SOCI_ORM_CLASS>::Loader(Orm& orm, const std::string& filter)
    : pimpl(std::make_unique<Impl>(orm, filter)) {}
template<> Loader<SOCI_ORM_CLASS>::Loader(Loader<SOCI_ORM_CLASS>&& other) { pimpl.swap(other.pimpl); }
template<> Loader<SOCI_ORM_CLASS>& Loader<SOCI_ORM_CLASS>::operator=(Loader<SOCI_ORM_CLASS>&& other) { pimpl.swap(other.pimpl); return *this; }
template<> Loader<SOCI_ORM_CLASS>::~Loader() {}
template<> std::unique_ptr<SOCI_ORM_CLASS> Loader<SOCI_ORM_CLASS>::fetch() { return pimpl->fetch(); }
template<> std::unique_ptr<SOCI_ORM_CLASS> Loader<SOCI_ORM_CLASS>::fetch_pk() { return pimpl->fetch_pk(); }
template<> const Loader<SOCI_ORM_CLASS>::ForeignKey* Loader<SOCI_ORM_CLASS>::fetch_fk() { return pimpl->fetch_fk(); }
template<> const Loader<SOCI_ORM_CLASS>::ForeignKey* Loader<SOCI_ORM_CLASS>::peek_fk() { return pimpl->peek_fk(); }
template<> const utils::AuditTransaction& Loader<SOCI_ORM_CLASS>::get_transaction_result() const { return pimpl->transaction_result; }
} /* !namespace soci_orm */