namespace soci_orm {

template<>
struct Saver<SOCI_ORM_CLASS>::PrimaryKey::Impl {
#define SOCI_ORM_PK(FIELD, NAME)        \
    std::vector<Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::orm_type> pk_ ## FIELD;
#define SOCI_ORM_FK(CLASS)              \
    Saver<CLASS>::Data fk_ ## CLASS;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
    std::vector<int64_t> autoindex_ ## FIELD;
#include "soci_xmacro.h"
};

template<>
template<>
void Saver<SOCI_ORM_CLASS>::PrimaryKey::append(const SOCI_ORM_CLASS& value, int64_t index) {
#define SOCI_ORM_PK(FIELD, NAME)        \
    pimpl->pk_ ## FIELD.push_back(Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value)));
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
    pimpl->autoindex_ ## FIELD.push_back(index);
#include "soci_xmacro.h"
}

#define SOCI_ORM_FK(CLASS)                                                          \
template<>                                                                          \
template<>                                                                          \
void Saver<SOCI_ORM_CLASS>::PrimaryKey::append(const CLASS& value, int64_t index) { \
    pimpl->fk_ ## CLASS.append(value, index);                                       \
}
#include "soci_xmacro.h"

template<>
void Saver<SOCI_ORM_CLASS>::PrimaryKey::exchange(soci::statement& stmt) const {
#define SOCI_ORM_PK(FIELD, NAME)        \
    stmt.exchange(soci::use(pimpl->pk_ ## FIELD, NAME)); std::cout << "Exchange Pk " NAME "\n";  
#define SOCI_ORM_FK(CLASS)              \
    pimpl->fk_ ## CLASS.exchange(stmt);
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
    stmt.exchange(soci::use(pimpl->autoindex_ ## FIELD, NAME));
#include "soci_xmacro.h"
}

template<>
struct Saver<SOCI_ORM_CLASS>::Data::Impl {
    Saver<SOCI_ORM_CLASS>::PrimaryKey primary_key;
    std::vector<utils::Action*> actions;
#define SOCI_ORM_VALUE(FIELD, NAME)     \
    std::vector<Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::orm_type> value_ ## FIELD;
#define SOCI_ORM_INLINE(FIELD)          \
    Saver<SOCI_ORM_ACCESSOR::FIELD>::Data inline_ ## FIELD;
#include "soci_xmacro.h"
};


template<> Saver<SOCI_ORM_CLASS>::PrimaryKey::PrimaryKey()
    : pimpl(std::make_unique<Impl>()) {}

template<>
void Saver<SOCI_ORM_CLASS>::Data::append_action(utils::Action* action) {
    if (action)
        pimpl->actions.push_back(action);
}

template<>
template<>
void Saver<SOCI_ORM_CLASS>::Data::append(const SOCI_ORM_CLASS& value, int64_t index) {
    pimpl->primary_key.append(value, index);
#define SOCI_ORM_VALUE(FIELD, NAME)     \
    pimpl->value_ ## FIELD.push_back(Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value)));
#define SOCI_ORM_INLINE(FIELD)          \
    pimpl->inline_ ## FIELD.append(value, index);
#include "soci_xmacro.h"
}

#define SOCI_ORM_FK(CLASS)                                                      \
template<>                                                                      \
template<>                                                                      \
void Saver<SOCI_ORM_CLASS>::Data::append(const CLASS& value, int64_t index) {   \
    pimpl->primary_key.append(value, index);                                    \
}
#include "soci_xmacro.h"

template<>
void Saver<SOCI_ORM_CLASS>::Data::exchange(soci::statement& stmt) const {
    pimpl->primary_key.exchange(stmt);
#define SOCI_ORM_VALUE(FIELD, NAME)     \
    stmt.exchange(soci::use(pimpl->value_ ## FIELD, NAME));
#define SOCI_ORM_INLINE(FIELD)          \
    pimpl->inline_ ## FIELD.exchange(stmt);
#include "soci_xmacro.h"
}


template<>
void Saver<SOCI_ORM_CLASS>::Data::execute(soci::statement& stmt, utils::Action futureAction) const {
    exchange(stmt);
    stmt.define_and_bind();
    stmt.execute(true);
    stmt.bind_clean_up();
    for (auto* action : pimpl->actions)
        *action = futureAction;
}

template<> Saver<SOCI_ORM_CLASS>::Data::Data()
    : pimpl(std::make_unique<Impl>()) {}

template<>
struct Saver<SOCI_ORM_CLASS>::Impl {
    std::map<utils::Action, Saver<SOCI_ORM_CLASS>::Data> data;
#define SOCI_ORM_COLLECTION(FIELD)  \
    Saver<std::remove_pointer_t<decltype(SOCI_ORM_ACCESSOR::FIELD)::value_type>> saver_ ## FIELD;
#include "soci_xmacro.h"
};

template<>
Saver<SOCI_ORM_CLASS>::Data* Saver<SOCI_ORM_CLASS>::append(Repository<SOCI_ORM_CLASS>* repository, const SOCI_ORM_CLASS& value, int64_t index) {
    Saver<SOCI_ORM_CLASS>::Data* proxy = nullptr;
    utils::Action action = utils::Action::INSERT;
    utils::Action* ref_action = nullptr;
    Repository<SOCI_ORM_CLASS>::Value* repo_value = nullptr;

    if (repository) {
        repo_value = &repository->find(value, index);
        ref_action = repo_value->get_action();
        action = *ref_action;
    }

    switch (action) {
        case utils::Action::INSERT:
            ++affected.inserted;
            proxy = &pimpl->data[action];
            proxy->append(value, index);
            proxy->append_action(ref_action);
            break;
        case utils::Action::UPDATE:
            ++affected.updated;
            proxy = &pimpl->data[action];
            proxy->append(value, index);
            proxy->append_action(ref_action);
            break;
        case utils::Action::MERGE:
            ++affected.merged;
            proxy = &pimpl->data[action];
            proxy->append(value, index);
            proxy->append_action(ref_action);
            break;
        case utils::Action::IGNORE:
            ++affected.ignored;
            break;
    }
#define SOCI_ORM_COLLECTION(FIELD)                                                              \
    {                                                                                           \
        using ValType = std::remove_pointer_t<decltype(SOCI_ORM_ACCESSOR::FIELD)::value_type>;  \
        int64_t auto_index = 0;                                                                 \
        Repository<ValType>* child_repo (repo_value ? &repo_value->FIELD : nullptr);            \
        for (const auto& child : SOCI_ORM_ACCESSOR::get_ ## FIELD(value)) {                     \
            if (auto* p = pimpl->saver_ ##  FIELD.append(child_repo, child, auto_index)) {      \
                p->append(value, auto_index);                                                   \
            }                                                                                   \
            ++auto_index;                                                                       \
        }                                                                                       \
    }
#include "soci_xmacro.h"
    return proxy;
}

template<>
utils::AuditTransaction Saver<SOCI_ORM_CLASS>::save(Orm& orm) {
    utils::AuditTransaction transactionResult;
    utils::Action action = utils::Action::UPDATE;
#define SOCI_ORM_ACTION(NEW_ACTION, UPDATE_ACTION)  \
    action = UPDATE_ACTION;
#include "soci_xmacro.h"
    {
        for (auto& it : pimpl->data) {
            switch (it.first) {
                case utils::Action::INSERT:
                {
                    auto stmt = ORM<SOCI_ORM_CLASS>::details::prepare_insert(orm.session);
                    it.second.execute(stmt, action);
                    break;
                }
                case utils::Action::UPDATE:
                {
                    auto stmt = ORM<SOCI_ORM_CLASS>::details::prepare_update(orm.session);
                    it.second.execute(stmt, action);
                    break;
                }
                case utils::Action::MERGE:
                {
                    auto stmt = ORM<SOCI_ORM_CLASS>::details::prepare_merge(orm.session);
                    it.second.execute(stmt, action);
                    break;
                }
                default: break;
            }
        }
        transactionResult[ORM<SOCI_ORM_CLASS>::details::get_table()] = affected;
    }
#define SOCI_ORM_COLLECTION(FIELD)                          \
    for (auto& child : pimpl->saver_ ## FIELD.save(orm))    \
        transactionResult[child.first] += child.second;
#include "soci_xmacro.h"
    return transactionResult;
}

template<> Saver<SOCI_ORM_CLASS>::Saver()
    : pimpl(std::make_unique<Impl>()) {}

template<> Saver<SOCI_ORM_CLASS>::~Saver() {}
template<> Saver<SOCI_ORM_CLASS>::Saver(Saver<SOCI_ORM_CLASS>&& other) { pimpl.swap(other.pimpl); }
template<> Saver<SOCI_ORM_CLASS>& Saver<SOCI_ORM_CLASS>::operator=(Saver<SOCI_ORM_CLASS>&& other) { pimpl.swap(other.pimpl); return *this; }

} /* !namespace soci_orm */
