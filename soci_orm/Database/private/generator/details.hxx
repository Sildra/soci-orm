#define SOCI_ORM_DETAILS  soci_orm::ORM<SOCI_ORM_CLASS>::details

template<>
const std::string& SOCI_ORM_DETAILS::get_table()
{
    static const std::string table = std::string()
#define SOCI_ORM_TABLE(NAME)    \
        .append(NAME)
#include "soci_xmacro.h"
    ;
    return table;
}

template<>
void SOCI_ORM_DETAILS::fetch_pk(std::vector<std::string>& pks)
{
#define SOCI_ORM_PK(FIELD, NAME)    \
    pks.push_back(NAME);
#define SOCI_ORM_INLINE(FIELD)      \
    ORM<decltype(SOCI_ORM_ACCESSOR::FIELD)>::details::fetch_pk(pks);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::fetch_fields(std::vector<std::string>& fields)
{
#define SOCI_ORM_VALUE(FIELD, NAME) \
    fields.push_back(NAME);
#define SOCI_ORM_INLINE(FIELD)      \
    ORM<decltype(SOCI_ORM_ACCESSOR::FIELD)>::details::fetch_fields(fields);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::fetch_fk(std::vector<std::string>& fks)
{
#define SOCI_ORM_PK(CLASS)      \
    ORM<CLASS>::details::fetch_pk(fks);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::create_columns(soci::session& sql, const std::string& original_table,
utils::AuditMigrationTable& audit)
{
    using AuditMigration = soci_orm::utils::AuditMigration;
#define SOCI_ORM_VALUE(FIELD, NAME)                                             \
    {                                                                           \
        auto m = audit.find(AuditMigration { NAME });                           \
        if (m != audit.end() && m->action == AuditMigration::Action::CREATED)   \
            sql.add_column(original_table, NAME, soci_orm::Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::db_type); \
    }
#define SOCI_ORM_INLINE(FIELD)                                                  \
    soci_orm::ORM<decltype(SOCI_ORM_ACCESSOR::FIELD)>::details::create_columns(sql, original_table, audit);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::remove_columns(soci::session& sql, const std::string& original_table,
    soci_orm::utils::AuditMigrationTable& audit)
{
    for (const auto& column : audit) {
        if (column.action == soci_orm::utils::AuditMigration::Action::DELETED)
            sql.drop_column(original_table, column.name);
    }
}

template<>
void SOCI_ORM_DETAILS::add_pk(soci::ddl_type& ddl)
{
#define SOCI_ORM_PK(FIELD, NAME)                                                            \
    ddl.column(NAME, soci_orm::Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::db_type,       \
        soci_orm::Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::db_type == soci::db_type::db_string ? 255 : 0);
#define SOCI_ORM_FK(CLASS)                                                                  \
    soci_orm::ORM<CLASS>::details::add_pk(ddl);
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                                                     \
    ddl.column(NAME, soci_orm::Affinity<int64_t>::db_type);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::add_pk_constraints(soci::ddl_type& ddl)
{
#define SOCI_ORM_FK(CLASS)  \
    soci_orm::ORM<CLASS>::details::add_fk_constraints(ddl);
#include "soci_xmacro.h"

    const auto& pk = get_pk();
    if (pk.empty())
        return;

    ddl.primary_key(get_table() + "_PK", soci_orm::utils::merge_string(", ", pk));
}

template<>
void SOCI_ORM_DETAILS::add_fk_constraints(soci::ddl_type& ddl)
{
    const auto& fk = get_pk();
    if (fk.empty())
        return;
    std::string columns = soci_orm::utils::merge_string(", ", fk);
    ddl.foreign_key(get_table() + "_FK", columns, get_table(), columns);
}

template<>
void SOCI_ORM_DETAILS::add_index(soci::session& sql, const std::string& name, const std::vector<std::string>& columns)
{
    sql << "create index " << name << " on " << get_table() << "(" << soci_orm::utils::merge_string(", ", columns) << ")";
}

template<>
void SOCI_ORM_DETAILS::migrate(soci::ddl_type& ddl, const std::string& originalTable, std::set<std::string>& currentColumns)
{
#define SOCI_ORM_VALUE(FIELD, NAME)                        \
    if (!currentColumns.erase(NAME))                       \
        ddl.column(NAME, Affinity<std::decay_t<decltype(SOCI_ORM_CLASS::FIELD)>>::db_type);
#define SOCI_ORM_INLINE(FIELD)                              \
    ORM<std::decay_t<decltype()>>::migrate(ddl, originalTable, currentColumns);
#define SOCI_ORM_PK(FIELD, NAME)                            \
    if (!currentColumns.erase(NAME)) {                     \
        ddl.column(NAME, Affinity<std::decay_t<decltype(SOCI_ORM_CLASS::FIELD)>>::db_type); \
    }
#define SOCI_ORM_FK(CLASS)  \
    ORM<CLASS>::details::add_primary_key(ddl, originalTable, currentColumns);
#include "soci_xmacro.h"
}

#undef SOCI_ORM_DETAILS
