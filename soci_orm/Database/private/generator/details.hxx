#define SOCI_ORM_DETAILS  soci_orm::ORM<SOCI_ORM_CLASS>::details

#if __cpp_if_constexpr
# define SOCI_ORM_IF_CONSTEXPR constexpr
#else
# define SOCI_ORM_IF_CONSTEXPR
#endif

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
    constexpr bool pk_as_fk =           false
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) || true
#define SOCI_ORM_PK(FIELD, NAME)        || true
#include "soci_xmacro.h"
        ;

#define SOCI_ORM_FK(CLASS)              \
    if SOCI_ORM_IF_CONSTEXPR (pk_as_fk) ORM<CLASS>::details::fetch_pk(pks);
#define SOCI_ORM_PK(FIELD, NAME)        \
    pks.push_back(NAME);
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
    pks.push_back(NAME);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::fetch_fields(std::vector<std::string>& fields)
{
    constexpr bool pk_as_fk =           false
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) || true
#define SOCI_ORM_PK(FIELD, NAME)        || true
#include "soci_xmacro.h"
        ;

#define SOCI_ORM_FK(CLASS)          \
    if SOCI_ORM_IF_CONSTEXPR (!pk_as_fk) ORM<CLASS>::details::fetch_pk(fields);
#define SOCI_ORM_VALUE(FIELD, NAME) \
    fields.push_back(NAME);
#define SOCI_ORM_INLINE(FIELD)      \
    ORM<decltype(SOCI_ORM_ACCESSOR::FIELD)>::details::fetch_pk(fields); \
    ORM<decltype(SOCI_ORM_ACCESSOR::FIELD)>::details::fetch_fields(fields);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::fetch_fk(std::vector<std::string>& fks)
{
#define SOCI_ORM_FK(CLASS)      \
    ORM<CLASS>::details::fetch_pk(fks);
#include "soci_xmacro.h"
}

template<>
std::string SOCI_ORM_DETAILS::build_insert_statement()
{
    std::stringstream ss;
    const auto& pks = get_pk();
    const auto& fields = get_fields();
    ss << "insert into " << get_table() << "(";

    for (auto& pk : pks) ss << pk << ",";
    for (auto& field : fields) ss << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ") values (";
    for (auto& pk : pks) ss << ":" << pk << ",";
    for (auto& field : fields) ss << ":" << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ")";
    return ss.str();
}

template<>
std::string SOCI_ORM_DETAILS::build_update_statement()
{
    const auto& pks = get_pk();
    if (pks.empty())
        return "";

    std::stringstream ss;
    const auto& fields = get_fields();
    ss << "update " << get_table() << " set ";

    for (auto& field : fields) ss << field << " = :" << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << " where 1=1 ";
    for (auto& pk : pks) ss << " and " << pk << " = :" << pk;

    return ss.str();
}

template<>
std::string SOCI_ORM_DETAILS::build_merge_statement_std(soci::session& sql)
{
    const auto& pks = get_pk();
    if (pks.empty())
        return insert_statement();

    std::stringstream ss;
    const auto& fields = get_fields();
    ss << "merge into " << get_table() << " TARGET using (select ";
    for (auto& field : pks)     ss << ":" << field << " AS " << field << ",";
    for (auto& field : fields)  ss << ":" << field << " AS " << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << sql.get_dummy_from_clause();

    ss << ") SOURCE on (";
    for (auto& field : pks)     ss << "TARGET." << field << " = SOURCE." << field << " and ";
    ss.seekp(-5, std::ios_base::end);

    ss << " when matched then update set ";
    for (auto& field : fields)  ss << "TARGET." << field << " = SOURCE." << field << ",";
    ss.seekp(-1, std::ios_base::end);

    ss << " when not matched then insert(";
    for (auto& field : pks)     ss << "TARGET." << field << ",";
    for (auto& field : fields)  ss << "TARGET." << field << ",";
    ss.seekp(-1, std::ios_base::end);
    
    ss << ") values (";

    for (auto& field : pks)     ss << "SOURCE." << field << ",";
    for (auto& field : fields)  ss << "SOURCE." << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ")";

    return ss.str();
}

template<>
std::string SOCI_ORM_DETAILS::build_merge_statement_sqlite()
{
    std::stringstream ss;
    const auto& pks = get_pk();
    const auto& fields = get_fields();
    ss << "insert into " << get_table() << "(";

    for (auto& pk : pks)        ss << pk << ",";
    for (auto& field : fields)  ss << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ") values (";
    for (auto& pk : pks)        ss << ":" << pk << ",";
    for (auto& field : fields)  ss << ":" << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ")";

    if (pks.empty())
        return ss.str();

    ss << " on conflict(";
    for (auto& pk : pks)        ss << pk << ",";
    ss.seekp(-1, std::ios_base::end);
    
    ss << ") do update set ";
    for (auto& field : fields)  ss << field << " = :" << field << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ' ';
    
    return ss.str();
}

template<>
void SOCI_ORM_DETAILS::create_columns(Orm& orm, const std::string& original_table,
    utils::AuditMigrationTable& audit)
{
    using AuditMigration = soci_orm::utils::AuditMigration;
#define SOCI_ORM_VALUE(FIELD, NAME)                                             \
    {                                                                           \
        auto m = audit.find(AuditMigration { NAME });                           \
        if (m != audit.end() && m->action == AuditMigration::Action::CREATED)   \
            orm.session.add_column(original_table, NAME, soci_orm::Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::db_type); \
    }
#define SOCI_ORM_INLINE(FIELD)                                                  \
    soci_orm::ORM<decltype(SOCI_ORM_ACCESSOR::FIELD)>::details::create_columns(orm, original_table, audit);
#include "soci_xmacro.h"
}

template<>
void SOCI_ORM_DETAILS::remove_columns(Orm& orm, const std::string& original_table,
    soci_orm::utils::AuditMigrationTable& audit)
{
    for (const auto& column : audit) {
        if (column.action == soci_orm::utils::AuditMigration::Action::DELETED)
            orm.session.drop_column(original_table, column.name);
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
void SOCI_ORM_DETAILS::add_index(Orm& orm, const std::string& name, const std::vector<std::string>& columns)
{
    orm.session << "create index " << name << " on " << get_table() << "(" << soci_orm::utils::merge_string(", ", columns) << ")";
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
    // ORM<CLASS>::details::add_primary_key(ddl, originalTable, currentColumns);
#include "soci_xmacro.h"
}

#undef SOCI_ORM_DETAILS
#undef SOCI_ORM_IF_CONSTEXPR