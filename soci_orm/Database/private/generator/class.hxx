
template<>
soci_orm::utils::AuditMigrationTables soci_orm::ORM<SOCI_ORM_CLASS>::migrate(Orm& orm)
{
    std::string table = details::get_table();
    using namespace soci_orm::utils;
    AuditMigrationTables result;
    AuditMigrationTable& migration = result[table];

    bool table_created = [&]()
        {
            std::string created_table;
            soci::statement st = (orm.session.prepare_table_names(), soci::into(created_table));
            st.execute();
            while (st.fetch()) {
                if (created_table == table)
                    return true;
            }
            return false;
        }();

    // Create table with PK and FK
    if (!table_created) {
        {
            auto ddl = orm.session.create_table(table);
            details::add_pk(ddl);
#define SOCI_ORM_VALUE(FIELD, NAME)         \
            ddl.column(NAME, soci_orm::Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::db_type);
#include "soci_xmacro.h"
            details::add_pk_constraints(ddl);
        }
        auto fk = details::get_fk();
        if (!fk.empty())
            details::add_index(orm, table + "_FK_INDEX", fk);
        migration.insert(AuditMigration { table, AuditMigration::Type::TABLE, AuditMigration::Action::CREATED });
    }

    // List all columns that needs to be present post-migration. PK and FK cannot be changed that way.
    for (const auto& pk : details::get_pk())
        migration.insert(AuditMigration { pk, AuditMigration::Type::PK, AuditMigration::Action::CREATION_NOT_SUPPORTED });
    for (const auto& fk : details::get_fk())
        migration.insert(AuditMigration { fk, AuditMigration::Type::FK, AuditMigration::Action::CREATION_NOT_SUPPORTED });
    for (const auto& field : details::get_fields())
        migration.insert(AuditMigration { field, AuditMigration::Type::COL, AuditMigration::Action::CREATED });

    // Remove columns that are already present or flag them for deletion.
    {
        soci::column_info ci;
        soci::statement st = (orm.session.prepare_column_descriptions(table), soci::into(ci));
        st.execute();
        while (st.fetch()) {
            auto inserted = migration.insert(AuditMigration { ci.name, AuditMigration::Type::COL, AuditMigration::Action::DELETED });
            if (!inserted.second)
                migration.erase(inserted.first);
        }
    }

    // Perform migration
    details::create_columns(orm, table, migration);
    details::remove_columns(orm, table, migration);

    if (migration.empty())
        result.erase(table);

#define SOCI_ORM_COLLECTION(FIELD)                                                                  \
    {                                                                                               \
        typedef std::decay_t<decltype(SOCI_ORM_ACCESSOR::FIELD)::value_type> ValueType;             \
        auto sub = soci_orm::ORM<ValueType>::migrate(orm);                                          \
        result.insert(std::make_move_iterator(sub.begin()), std::make_move_iterator(sub.end()));    \
    }
#include "soci_xmacro.h"

    return result;
}