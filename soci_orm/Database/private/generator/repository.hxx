#define SOCI_ORM_REPOSITORY Repository<SOCI_ORM_CLASS>

namespace soci_orm {
template<>
struct SOCI_ORM_REPOSITORY::Key {
#define SOCI_ORM_PK(FIELD, NAME)                        \
    Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::orm_type FIELD;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
    int64_t FIELD;
#include "soci_xmacro.h"

    Key(const SOCI_ORM_CLASS& value, int64_t index) {
#define SOCI_ORM_PK(FIELD, NAME)                        \
        this.FIELD = Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value));
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        this.FIELD = index;
#include "soci_xmacro.h"
    }

    bool operator<(const Key<SOCI_ORM_CLASS>& other) const {
        constexpr bool tie_field = true;
        return std::tie(                tie_field
#define SOCI_ORM_PK(FIELD, NAME)        , FIELD
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) , FIELD
#include "soci_xmacro.h"
        ) < std::tie(                   tie_field
#define SOCI_ORM_PK(FIELD, NAME)        , other.FIELD
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) , other.FIELD
#include "soci_xmacro.h"
        );
    }
};

template<>
struct SOCI_ORM_REPOSITORY::Value {
    utils::Action action {
#define SOCI_ORM_ACTION(INITIAL_ACTION, SAVING_ACTION)  \
        INITIAL_ACTION
#include "soci_xmacro.h"
    };
#define SOCI_ORM_COLLECTION(FIELD)                      \
    Repository<decltype(SOCI_ORM_ACCESSOR::FIELD)> FIELD;
#include "soci_xmacro.h"
    utils::Action* get_action() { return &action; }
};

template<>
struct SOCI_ORM_REPOSITORY::Impl {
    std::map<Key, Value> repository;
    Value& find(const SOCI_ORM_CLASS& value, int64_t index) {
        return repository[Key(value, index)];
    }
};

template<> SOCI_ORM_REPOSITORY::Repository()
    : pimpl(std::make_unique<SOCI_ORM_REPOSITORY::Impl>()) { }
template<> SOCI_ORM_REPOSITORY::~Repository() { }
template<> SOCI_ORM_REPOSITORY::Repository(SOCI_ORM_REPOSITORY&& other) { pimpl.swap(other.pimpl); }
template<> SOCI_ORM_REPOSITORY::Repository() { pimpl.swap(other.pimpl); return *this; }

} /* !namespace soci_orm */
