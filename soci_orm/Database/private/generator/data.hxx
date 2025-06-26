/*  SINGLE  */

template<>
struct SOCI_ORM_SINGLE::Impl {
#define SOCI_ORM_FK(CLASS)                              \
    soci_orm::Single<CLASS> fk_ ## CLASS;
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
    Affinity<CONVERTOR>::orm_type pk_ ## FIELD;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
    int64_t autoindex_ ## FIELD;
#include "soci_xmacro.h"

    int compare(const SOCI_ORM_SINGLE::Impl& other) const {
#define SOCI_ORM_FK(CLASS)                              \
        if (int cmp = fk_ ## CLASS.compare(other.fk_ ## CLASS)) return cmp;
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)         \
        if (int cmp = soci_orm::utils::compare(pk_ ## FIELD, other.pk_ ## FIELD)) return cmp;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        if (int cmp = soci_orm::utils::compare(autoindex_ ## FIELD, other.autoindex_ ## FIELD)) return cmp;
#include "soci_xmacro.h"
        return 0;
    }
};

template<> SOCI_ORM_SINGLE::Single() : pimpl { std::make_unique<Impl>() } {}
template<> SOCI_ORM_SINGLE::~Single() {}
template<> SOCI_ORM_SINGLE::Single(Single&& other) noexcept { pimpl.swap(other.pimpl); }

template<> int SOCI_ORM_SINGLE::compare(const Single& other) const {
    return pimpl->compare(*other.pimpl);
}
#define SOCI_ORM_FK(CLASS)                              \
template<> template<> const soci_orm::Single<CLASS>& SOCI_ORM_SINGLE::get() const { return pimpl->fk_ ## CLASS; }
#include "soci_xmacro.h"

/*  BULK - PK  */

template<>
struct SOCI_ORM_BULK::PrimaryKey::Impl {
#define SOCI_ORM_FK(CLASS)                              \
    soci_orm::Bulk<CLASS>::PrimaryKey fk_ ## CLASS;
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
    std::vector<Affinity<CONVERTOR>::orm_type> pk_ ## FIELD;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
    std::vector<int64_t> autoindex_ ## FIELD;
#include "soci_xmacro.h"

    void resize(size_t size) {
#define SOCI_ORM_FK(CLASS)                              \
        fk_ ## CLASS.resize(size);
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
        pk_ ## FIELD.resize(size);
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        autoindex_ ## FIELD.resize(size);
#include "soci_xmacro.h"
    }

    void append(const SOCI_ORM_CLASS& value, int64_t index) {
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
        pk_ ## FIELD.push_back(Affinity<CONVERTOR>::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value)));
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        autoindex_ ## FIELD.push_back(index);
#include "soci_xmacro.h"
    }

#define SOCI_ORM_FK(CLASS)                              \
    void append(const CLASS& value, int64_t index) {    \
        fk_ ## CLASS.append(value, index);              \
    }
#include "soci_xmacro.h"

    void get(SOCI_ORM_CLASS& value, size_t index) {
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
        Affinity<CONVERTOR>::from_bulk(pk_ ## FIELD[index], SOCI_ORM_ACCESSOR::mutable_get_ ## FIELD(value));
#include "soci_xmacro.h"
    }

    void get(SOCI_ORM_SINGLE& value, size_t index) {
        auto& val = *value.pimpl;
#define SOCI_ORM_FK(CLASS)                              \
        fk_ ## CLASS.get(val.fk_ ## CLASS, index);
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
        val.pk_ ## FIELD = pk_ ## FIELD[index];
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        val.autoindex_ ## FIELD = autoindex_ ## FIELD[index];
#include "soci_xmacro.h"
    }

    void into(soci::statement& stmt, std::vector<soci::indicator>& indicators) {
#define SOCI_ORM_FK(CLASS)                              \
        fk_ ## CLASS.into(stmt, indicators);
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
        stmt.exchange(soci::into(pk_ ## FIELD, indicators));
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        stmt.exchange(soci::into(autoindex_ ## FIELD, indicators));
#include "soci_xmacro.h"
    }

    void use(soci::statement& stmt) const {
#define SOCI_ORM_FK(CLASS)                              \
        fk_ ## CLASS.use(stmt);
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)        \
        stmt.exchange(soci::use(pk_ ## FIELD, NAME));
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        stmt.exchange(soci::use(autoindex_ ## FIELD, NAME));
#include "soci_xmacro.h"
    }
};

template<> SOCI_ORM_BULK::PrimaryKey::PrimaryKey() : pimpl(std::make_unique<Impl>()) {}
template<> SOCI_ORM_BULK::PrimaryKey::~PrimaryKey() {}

template<> void SOCI_ORM_BULK::PrimaryKey::resize(size_t size) {
    pimpl->resize(size);
}   
template<> template<> void SOCI_ORM_BULK::PrimaryKey::append(const SOCI_ORM_CLASS& value, int64_t index) {
    pimpl->append(value, index);
}
#define SOCI_ORM_FK(CLASS)                                                                          \
template<> template<> void SOCI_ORM_BULK::PrimaryKey::append(const CLASS& value, int64_t index) {   \
    pimpl->append(value, index);                                                                    \
}
#include "soci_xmacro.h"
template<> void SOCI_ORM_BULK::PrimaryKey::get(SOCI_ORM_CLASS& value, size_t index) {
    pimpl->get(value, index);
}
template<> void SOCI_ORM_BULK::PrimaryKey::get(SOCI_ORM_SINGLE& value, size_t index) {
    pimpl->get(value, index);
}
template<>
void SOCI_ORM_BULK::PrimaryKey::into(soci::statement& stmt, std::vector<soci::indicator>& indicators) {
    pimpl->into(stmt, indicators);
}
template<>
void SOCI_ORM_BULK::PrimaryKey::use(soci::statement& stmt) const {
    pimpl->use(stmt);
}

/*  BULK  */

template<>
struct SOCI_ORM_BULK::Impl {
    SOCI_ORM_BULK::PrimaryKey::Impl primary_key;
    std::vector<utils::Action*> actions;
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)         \
    std::vector<Affinity<CONVERTOR>::orm_type> value_ ## FIELD;
#define SOCI_ORM_INLINE(FIELD)                              \
    Saver<SOCI_ORM_ACCESSOR::FIELD>::Data inline_ ## FIELD;
#include "soci_xmacro.h"

    void resize(size_t size) {
        primary_key.resize(size);
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)         \
        value_ ## FIELD.resize(size);
#define SOCI_ORM_INLINE(FIELD)                              \
        inline_ ## FIELD.resize(size);
#include "soci_xmacro.h"
    }

    void append_action(utils::Action* action) {
        if (action)
            actions.push_back(action);
    }

    void append(const SOCI_ORM_CLASS& value, int64_t index) {
        primary_key.append(value, index);
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)         \
        value_ ## FIELD.push_back(soci_orm::Affinity<CONVERTOR>::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value)));
#define SOCI_ORM_INLINE(FIELD)                              \
        inline_ ## FIELD.append(value, index);
#include "soci_xmacro.h"
    }

#define SOCI_ORM_FK(CLASS)                                  \
    void append(const CLASS& value, int64_t index) {        \
        primary_key.append(value, index);                   \
    }
#include "soci_xmacro.h"

    void get(SOCI_ORM_CLASS& value, size_t index) {
        primary_key.get(value, index);
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)         \
        soci_orm::Affinity<CONVERTOR>::from_bulk(value_ ## FIELD[index], SOCI_ORM_ACCESSOR::mutable_get_ ## FIELD(value));
#define SOCI_ORM_INLINE(FIELD)                              \
        inline_ ## FIELD.get(value, index);
#include "soci_xmacro.h"
    }

    void get(SOCI_ORM_SINGLE& value, size_t index) {
        primary_key.get(value, index);
    }

    void into(soci::statement& stmt, std::vector<soci::indicator>& indicators) {
        primary_key.into(stmt, indicators);
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)         \
        stmt.exchange(soci::into(value_ ## FIELD, indicators));
#define SOCI_ORM_INLINE(FIELD)                              \
        inline_ ## FIELD.into(stmt, indicators);
#include "soci_xmacro.h"
    }
    void use(soci::statement& stmt) const {
        primary_key.use(stmt);
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)         \
        stmt.exchange(soci::use(value_ ## FIELD, NAME));
#define SOCI_ORM_INLINE(FIELD)                              \
        inline_ ## FIELD.use(stmt);
#include "soci_xmacro.h"
    }
};

template<> SOCI_ORM_BULK::Bulk() : pimpl(std::make_unique<Impl>()) {}
template<> SOCI_ORM_BULK::~Bulk() {};

template<> void SOCI_ORM_BULK::resize(size_t size) {
    pimpl->resize(size);
}
template<> void SOCI_ORM_BULK::append_action(utils::Action* action) {
    pimpl->append_action(action);
}
template<> template<> void SOCI_ORM_BULK::append(const SOCI_ORM_CLASS& value, int64_t index) {
    pimpl->append(value, index);
}
#define SOCI_ORM_FK(CLASS)                                                              \
template<> template<> void SOCI_ORM_BULK::append(const CLASS& value, int64_t index) {   \
    pimpl->append(value, index);                                                        \
}
#include "soci_xmacro.h"
template<> void SOCI_ORM_BULK::get(SOCI_ORM_CLASS& value, size_t index) {
    pimpl->get(value, index);
}
template<> void SOCI_ORM_BULK::get(SOCI_ORM_SINGLE& value, size_t index) {
    pimpl->get(value, index);
}
template<> void SOCI_ORM_BULK::into(soci::statement& stmt, std::vector<soci::indicator>& indicators) {
    pimpl->into(stmt, indicators);
}
template<> void SOCI_ORM_BULK::use(soci::statement& stmt) const {
    pimpl->use(stmt);
}

template<> void SOCI_ORM_BULK::save(soci::statement& stmt, utils::Action futureAction) const {
    use(stmt);
    stmt.define_and_bind();
    stmt.execute(true);
    stmt.bind_clean_up();
    for (auto* action : pimpl->actions)
        *action = futureAction;
}
