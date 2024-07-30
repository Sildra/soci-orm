namespace soci_orm {

struct SaverData<SOCI_ORM_CLASS>::Impl {
    struct PrimaryKey {
#define SOCI_ORM_PK(FIELD, NAME)        \
        std::vector<Affinity<SOCI_ORM_ACESSOR::FIELD>::orm_type> pk_ ## FIELD;
#define SOCI_ORM_FK(CLASS)              \
        SaverData<CLASS> fk_ ## CLASS;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
        std::vector<int64_t> autoindex_ ## FIELD;
#include "soci_xmacro.h"

        void append(const SOCI_ORM_CLASS& value, int64_t index) {
#define SOCI_ORM_PK(FIELD, NAME)        \
            pk_ ## FIELD.push_back(Affinity<SOCI_ORM_ACCESSOR::FIELD>::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value)));
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
            autoindex_ ## FIELD.push_back(index);
#include "soci_xmacro.h"
        }

#define SOCI_ORM_FK(CLASS)                                  \
        void append(const CLASS& value, int64_t index) {    \
            fk_ ## CLASS.append(value, index);              \
        }
#include "soci_xmacro.h"

        void exchange(soci::statement& stmt) const {
#define SOCI_PK(FIELD, NAME)    \
            stmt.exchange(oci::use(pk_ ## FIELD, NAME));
#define SOCI_ORM_FK(CLASS)      \
            fk_ ## CLASS.exchange(stmt);
#include "soci_xmacro.h"
        }
    };

    PrimaryKey primary_key;
    std::vector<utils::Action*> actions;
#define SOCI_ORM_VALUE(FIELD, NAME)     \
    std::vector<Affinity<SOCI_ORM_ACCESSOR::FIELD>::orm_type> value_ ## FIELD;
#define SOCI_ORM_INLINE(FIELD)          \
    SaverData<SOCI_ORM_ACCESSOR::FIELD> inline_ ## FIELD;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
    std::vector<int64_t> autoindex_ ## FIELD;
#include "soci_xmacro.h"

    void append_action(utils::Action* action) {
        if (action)
            actions.push_back(action);
    }

    template<>
    void append(const SOCI_ORM_CLASS& value, int64_t index) {
        primaryKey.append(value, index);
#define SOCI_ORM_VALUE(FIELD, NAME)     \
        value_ ## FIELD.push_back(Accessor::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value)));
#define SOCI_ORM_INLINE(FIELD)          \
        inline_ ## FIELD.append(value, index);
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
        autoindex_ ## FIELD.push_back(index);
#include "soci_xmacro.h"
    }

#define SOCI_ORM_FK(CLASS)                              \
    template<>                                          \
    void append(const CLASS& value, int64_t index) {    \
        primaryKey.append(value, index);                \
    }
#include "soci_xmacro.h"

    void exchange(soci::statement& stmt) const {
        primaryKey.exchange(stmt);
#define SOCI_ORM_VALUE(FIELD, NAME)     \
    stmt.exchange(soci::use(value_ ## FIELD, NAME));
#define SOCI_ORM_INLINE(FIELD)          \
    inline_ ## FIELD.exchange(stmt);
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
    stmt.exchange(soci::use(autoindex_ ## FIELD, NAME));
#include "soci_xmacro.h"
    }
}

template<>
struct Saver<SOCI_ORM_CLASS>::Impl {
    std::map<utils::Action, SaverData<SOCI_ORM_CLASS>> data;
#define SOCI_ORM_COLLECTION(FIELD)  \
    Saver<std::ermove_pointer_t<SOCI_ORM_ACCESSOR::FIELD::value_type>> saver_ ## FIELD;
#include "soci_xmacro.h"
};

template<>
SaverData* Saver<SOCI_ORM_CLASS>::append(Repository<SOCI_ORM_CLASS>* repository, const SOCI_ORM_CLASS& value, int64_t index) {
    SaverData<SOCI_ORM_CLASS>* proxy = nullptr;
    utils::Action action = utils::Action::INSERT;
    utils::Action* refAction = nullptr;
    Repository<SOCI_ORM_CLASS>::Value* repo_value = nullptr;

    if (repository) {
        repo_value = &repository->find(value, index);
        refAction = repo_value->getAction();
        action = *refAction;
    }

    switch (action) {
        case utils::Action::INSERT:
            ++affected.inserted;
            proxy = &pimpl->data[action];
            proxy->append(value, index);
            proxy->appendAction(refAction);
            break;
        case utils::Action::UPDATE:
            ++affected.updated;
            proxy = &pimpl->data[action];
            proxy->append(value, index);
            proxy->appendAction(refAction);
            break;
        case utils::Action::MERGE:
            ++affected.merged;
            proxy = &pimpl->data[action];
            proxy->append(value, index);
            proxy->appendAction(refAction);
            break;
        case utils::Action::IGNORE:
            ++affected.ignored;
            break;
    }
#define SOCI_ORM_COLLECTION(FIELD)                                                          \
    {                                                                                       \
        using ValType = std::remove_pointer_t<SOCI_ORM_ACCESSOR::FIELD::value_type>;        \
        int64_t auto_index = 0;                                                             \
        Repository<ValType>* child_repo (repo_value ? &repo_value->FIELD : nullptr);        \
        for (const auto& child : SOCI_ORM_ACCESSOR::get_ ## FIELD(value)) {                 \
            if (auto* p = pimpl->saver_ ##  FIELD.append(child_repo, child, auto_index) {   \
                p->append(value, auto_index);                                               \
            }                                                                               \
            ++auto_index;                                                                   \
        }                                                                                   \
    }
#include "soci_xmacro.h"
    return proxy;
}

template<> Saver<SOCI_ORM_CLASS>::Saver
    : pimpl(std::make_unique<Saver<SOCI_ORM_CLASS>>()) {}

template<> Saver<SOCI_ORM_CLASS>::~Saver() {}
template<> Saver<SOCI_ORM_CLASS>::Saver(Saver<SOCI_ORM_CLASS>&& other) { pimpl.swap(other.pimpl); }
template<> Saver<SOCI_ORM_CLASS>& Saver<SOCI_ORM_CLASS>::operator=(Saver<SOCI_ORM_CLASS>&& other) { pimpl.swap(other.pimpl); return *this; }

} /* !namespace soci_orm */
