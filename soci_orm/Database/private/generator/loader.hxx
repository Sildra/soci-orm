#ifndef SOCI_ORM_LOADER_COLLECTION
#define SOCI_ORM_LOADER_COLLECTION
template<typename ParentType, typename ChildType, typename Data, typename Functor, typename TransactionResult>
void process_collection(soci_orm::Orm& orm, const std::string& filter, Data& data, Functor mutable_get, TransactionResult& transaction_result) {
    soci_orm::Loader<ChildType> loader(orm, filter);
    for (auto& parent : data) {
        auto& coll = mutable_get(*parent.second);
        auto inserter = std::inserter(coll, coll.end());
        while (!loader.end()) {
            const auto& pk = loader.peek_pk().template get<ParentType>();
            auto cmp = pk.compare(parent.first);
            if (cmp == 0) {
                std::unique_ptr<ChildType> fetched = loader.fetch();
                soci_orm::utils::smart_inserter<decltype(inserter)>::template insert<ChildType>(inserter, std::move(fetched));
            }
            else if (cmp < 0)
                loader.next();
            else
                break;
        }
    }
    for (const auto& child : loader.get_transaction_result())
        transaction_result[child.first] += child.second;
}
#endif

template<>
struct SOCI_ORM_LOADER::Impl {
    std::map<SOCI_ORM_SINGLE, std::unique_ptr<SOCI_ORM_CLASS>> data;
    soci_orm::utils::AuditTransaction transaction_result;

    mutable decltype(data.begin()) data_it;
    soci_orm::utils::AuditRows& current_transaction = transaction_result[SOCI_ORM_DETAILS::get_table()];

    Impl(soci_orm::Orm& orm, std::string filter) {
        {
            using CD = soci_orm::utils::ColumnDefinition;
            std::string order(" order by ");
            order.append(soci_orm::utils::merge_string(",", SOCI_ORM_DETAILS::get_columns_definition(),
                [](const CD& v){ return (v.type == CD::Type::FK || v.type == CD::Type::PK || v.type == CD::Type::AUTOINDEX) ? v.name : ""; }));
            soci::statement stmt = SOCI_ORM_DETAILS::prepare_select(orm.session, filter + order);
            
            auto start_time = soci_orm::Orm::chrono::now();
            size_t prefetch_size = orm.fetch_size;
            SOCI_ORM_BULK bulk;
            std::vector<soci::indicator> indicators(prefetch_size);
            bulk.resize(prefetch_size);
            bulk.into(stmt, indicators);
            stmt.define_and_bind();

            stmt.execute();
            while (stmt.fetch()) {
                for (size_t i = 0; i < indicators.size(); ++i) {
                    ++current_transaction.ignored;
                    std::unique_ptr<SOCI_ORM_CLASS> value = std::make_unique<SOCI_ORM_CLASS>();
                    SOCI_ORM_SINGLE pk;
                    bulk.get(*value, i);
                    bulk.get(pk, i);
                    data.emplace_hint(data.end(), std::move(pk), std::move(value));
                }
            }
            auto end_time = soci_orm::Orm::chrono::now();
            current_transaction.elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        }
        data_it = data.begin();
        if (data.size() == 0) {
            return;
        }

#define SOCI_ORM_COLLECTION(FIELD)                                                                              \
        {                                                                                                       \
            using ChildType = std::remove_pointer_t<SOCI_ORM_ACCESSOR::FIELD::value_type>;                      \
            auto mutable_get = SOCI_ORM_ACCESSOR::mutable_get_ ## FIELD;                                        \
            using Functor = decltype(mutable_get);                                                              \
            process_collection<SOCI_ORM_CLASS, ChildType>(orm, filter, data, mutable_get, transaction_result);  \
        }
#include "soci_xmacro.h"
    }


    std::unique_ptr<SOCI_ORM_CLASS> fetch() {
        if (!end()) {
            auto val = std::move(data_it->second);
            next();
            --current_transaction.ignored;
            ++current_transaction.loaded;
            return val;
        }
        return nullptr;
    }
    
    const SOCI_ORM_SINGLE& peek_pk() const { return data_it->first; }
    bool end() const { return data_it == data.end(); }
    void next() { ++data_it; }
};

template<> SOCI_ORM_LOADER::Loader(Orm& orm, const std::string& filter) : pimpl { std::make_unique<Impl>(orm, filter) } {}
template<> SOCI_ORM_LOADER::~Loader() {}
template<> SOCI_ORM_LOADER::Loader(SOCI_ORM_LOADER&& other) { pimpl.swap(other.pimpl); }
template<> SOCI_ORM_LOADER& SOCI_ORM_LOADER::operator=(SOCI_ORM_LOADER&& other) { pimpl.swap(other.pimpl); return *this; }
template<> std::unique_ptr<SOCI_ORM_CLASS> SOCI_ORM_LOADER::fetch() { return pimpl->fetch(); }
template<> const SOCI_ORM_SINGLE& SOCI_ORM_LOADER::peek_pk() const { return pimpl->peek_pk(); }
template<> bool SOCI_ORM_LOADER::end() const { return pimpl->end(); }
template<> void SOCI_ORM_LOADER::next() { pimpl->next(); }

template<> const soci_orm::utils::AuditTransaction& SOCI_ORM_LOADER::get_transaction_result() const { return pimpl->transaction_result; }
