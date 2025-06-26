template<>
struct SOCI_ORM_SAVER::Impl {
    std::map<utils::Action, SOCI_ORM_BULK> data;
#define SOCI_ORM_COLLECTION(FIELD)  \
    soci_orm::Saver<std::remove_pointer_t<SOCI_ORM_ACCESSOR::FIELD::value_type>> saver_ ## FIELD;
#include "soci_xmacro.h"
};

template<>
SOCI_ORM_BULK* SOCI_ORM_SAVER::append(SOCI_ORM_REPOSITORY* repository, const SOCI_ORM_CLASS& value, int64_t index) {
    SOCI_ORM_BULK* proxy = nullptr;
    utils::Action action = utils::Action::INSERT;
    utils::Action* ref_action = nullptr;
    SOCI_ORM_REPOSITORY::Value* repo_value = nullptr;

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
        using ValType = std::remove_pointer_t<SOCI_ORM_ACCESSOR::FIELD::value_type>;            \
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
soci_orm::utils::AuditTransaction SOCI_ORM_SAVER::save(Orm& orm) {
    soci_orm::utils::AuditTransaction transactionResult;
    soci_orm::utils::Action action = soci_orm::utils::Action::UPDATE;
#define SOCI_ORM_ACTION(NEW_ACTION, UPDATE_ACTION)  \
    action = UPDATE_ACTION;
#include "soci_xmacro.h"
    {
        bool has_executed = false;
        auto start_time = Orm::chrono::now();
        for (auto& it : pimpl->data) {
            switch (it.first) {
                case soci_orm::utils::Action::INSERT:
                {
                    has_executed = true;
                    auto stmt = SOCI_ORM_DETAILS::prepare_insert(orm.session);
                    it.second.save(stmt, action);
                    break;
                }
                case soci_orm::utils::Action::UPDATE:
                {
                    has_executed = true;
                    auto stmt = SOCI_ORM_DETAILS::prepare_update(orm.session);
                    it.second.save(stmt, action);
                    break;
                }
                case soci_orm::utils::Action::MERGE:
                {
                    has_executed = true;
                    auto stmt = SOCI_ORM_DETAILS::prepare_merge(orm.session);
                    it.second.save(stmt, action);
                    break;
                }
                default: break;
            }
        }
        if (has_executed) {
            // Orm commit ?
            auto end_time = Orm::chrono::now();
            affected.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            transactionResult[SOCI_ORM_DETAILS::get_table()] = affected;
        }
    }
#define SOCI_ORM_COLLECTION(FIELD)                          \
    for (auto& child : pimpl->saver_ ## FIELD.save(orm))    \
        transactionResult[child.first] += child.second;
#include "soci_xmacro.h"
    return transactionResult;
}

template<> SOCI_ORM_SAVER::Saver()
    : pimpl(std::make_unique<Impl>()) {}

template<> SOCI_ORM_SAVER::~Saver() {}
template<> SOCI_ORM_SAVER::Saver(SOCI_ORM_SAVER&& other) { pimpl.swap(other.pimpl); }
template<> SOCI_ORM_SAVER& SOCI_ORM_SAVER::operator=(SOCI_ORM_SAVER&& other) { pimpl.swap(other.pimpl); return *this; }
