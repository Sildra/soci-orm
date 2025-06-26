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
const soci_orm::utils::ColumnsDefinition& SOCI_ORM_DETAILS::get_columns_definition()
{
    static soci_orm::utils::ColumnsDefinition columns_definition = []() {
        soci_orm::utils::ColumnsDefinition result;
        using CD = soci_orm::utils::ColumnDefinition;
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)                                             \
        result.push_back(CD { NAME, CD::Type::COL, Affinity<CONVERTOR>::db_type, Affinity<CONVERTOR>::db_size } );
#define SOCI_ORM_PK_FULL(CONVERTOR, FIELD, NAME)                                                \
        result.push_back(CD { NAME, CD::Type::PK, Affinity<CONVERTOR>::db_type, Affinity<CONVERTOR>::db_size } );
#define SOCI_ORM_INLINE(FIELD)                                                                  \
        for (const auto& def : ORM<SOCI_ORM_ACCESSOR::FIELD>::details::get_columns_definition()) { result.push_back(def); }
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                                                         \
        result.push_back(CD { NAME, CD::Type::AUTOINDEX, Affinity<int64_t>::db_type, 0 } );
#define SOCI_ORM_FK(CLASS)                                                                      \
        for (const auto& def : ORM<CLASS>::details::get_columns_definition()) {                 \
            if (def.type == CD::Type::PK) {                                                     \
                result.push_back(CD { def.name, CD::Type::FK, def.db_type, def.db_size } );     \
            }                                                                                   \
        }
#include "soci_xmacro.h"
        std::stable_sort(result.begin(), result.end(), [](const CD& a, const CD& b) { return a.type < b.type; });
        return result;
    }();
    return columns_definition;
}


template<>
std::string SOCI_ORM_DETAILS::build_insert_statement()
{
    std::stringstream ss;
    const auto& definition = get_columns_definition();
    ss << "insert into " << get_table() << "(";

    for (auto& field : definition) ss << field.name << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ") values (";
    for (auto& field : definition) ss << ":" << field.name << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ")";
    return ss.str();
}

template<>
std::string SOCI_ORM_DETAILS::build_update_statement()
{
    const auto& definition = get_columns_definition();
    if (definition[0].type == utils::ColumnDefinition::Type::COL)
        return "";

    std::stringstream ss;
    ss << "update " << get_table() << " set ";

    for (auto& field : definition) {
        if (field.type == utils::ColumnDefinition::Type::COL) {
            ss << field.name << " = :" << field.name << ",";
        }
    }
    ss.seekp(-1, std::ios_base::end);
    ss << " where 1=1 ";
    for (auto& field : definition) {
        if (field.type != utils::ColumnDefinition::Type::COL) {
            ss << field.name << " = :" << field.name << ",";
        }
    }

    return ss.str();
}

template<>
std::string SOCI_ORM_DETAILS::build_merge_statement_std(soci::session& sql)
{
    const auto& definition = get_columns_definition();
    if (definition[0].type == utils::ColumnDefinition::Type::COL)
        return insert_statement();

    std::stringstream ss;
    ss << "merge into " << get_table() << " TARGET using (select ";
    for (auto& field : definition)  ss << ":" << field.name << " AS " << field.name << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << sql.get_dummy_from_clause();

    ss << ") SOURCE on (";
    for (auto& field : definition) {
        if (field.type != utils::ColumnDefinition::Type::COL) {
            ss << "TARGET." << field.name << " = SOURCE." << field.name << " and ";
        }
    }
    ss.seekp(-5, std::ios_base::end);

    ss << " when matched then update set ";
    for (auto& field : definition) {
        if (field.type == utils::ColumnDefinition::Type::COL) {
            ss << "TARGET." << field.name << " = SOURCE." << field.name << ",";
        }
    }
    ss.seekp(-1, std::ios_base::end);

    ss << " when not matched then insert(";
    for (auto& field : definition)  ss << "TARGET." << field.name << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ") values (";
    for (auto& field : definition)  ss << "SOURCE." << field.name << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ")";

    return ss.str();
}

template<>
std::string SOCI_ORM_DETAILS::build_merge_statement_sqlite()
{
    std::stringstream ss;
    const auto& definition = get_columns_definition();
    ss << "insert into " << get_table() << "(";

    for (auto& field : definition)  ss << field.name << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ") values (";
    for (auto& field : definition)  ss << ":" << field.name << ",";
    ss.seekp(-1, std::ios_base::end);
    ss << ")";

    if (definition[0].type == utils::ColumnDefinition::Type::COL)
        return ss.str();

    ss << " on conflict(";
    for (auto& field : definition) {
        if (field.type != utils::ColumnDefinition::Type::COL) {
            ss << field.name << ",";
        }
    }
    ss.seekp(-1, std::ios_base::end);
    
    ss << ") do update set ";
    for (auto& field : definition) {
        if (field.type == utils::ColumnDefinition::Type::COL) {
            ss << field.name << " = :" << field.name << ",";
        }
    }
    ss.seekp(-1, std::ios_base::end);
    ss << ' ';
    
    return ss.str();
}

template<>
void SOCI_ORM_DETAILS::add_index(Orm& orm, const std::string& name, const std::vector<std::string>& columns)
{
    orm.session << "create index " << name << " on " << get_table() << "(" << soci_orm::utils::merge_string(", ", columns) << ")";
}
