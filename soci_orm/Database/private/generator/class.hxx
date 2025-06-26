template<>
soci_orm::utils::TablesMigration SOCI_ORM_MAIN::migrate(Orm& orm)
{
    std::string table = details::get_table();
    using namespace soci_orm::utils;
    TablesMigration result;
    auto& migration = result[table];

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
        auto ddl = orm.session.create_table(table);
        std::string pk_constraint;
        std::string fk_constraint;
        for (const auto& column : details::get_columns_definition()) {
            if (column.type != ColumnDefinition::Type::COL)
                pk_constraint += column.name + ",";
            if (column.type == ColumnDefinition::Type::FK)
                fk_constraint += column.name + ",";
            ddl.column(column.name, column.db_type, column.db_size);
        }
        if (!pk_constraint.empty()) {
            pk_constraint.pop_back();
            ddl.primary_key(table + "_PK", pk_constraint);
        }
        if (!fk_constraint.empty()) {
            fk_constraint.pop_back();
#define SOCI_ORM_FK(CLASS)    \
            ddl.foreign_key(table + "_FK", fk_constraint, ORM<CLASS>::details::get_table(), fk_constraint);
#include "soci_xmacro.h"
        }
    }

    // Remove columns that are already present or flag them for deletion.

    for (const auto& column : details::get_columns_definition()) {
        auto target_action = (column.type == ColumnDefinition::Type::COL ? ColumnMigration::Action::CREATED : ColumnMigration::Action::NO_ACTION);
        migration.insert( ColumnMigration { column, target_action });
    }
    {
        soci::column_info ci;
        soci::statement st = (orm.session.prepare_column_descriptions(table), soci::into(ci));
        st.execute();
        while (st.fetch()) {
            auto inserted = migration.insert(ColumnMigration { ColumnDefinition { ci.name, ColumnDefinition::Type::COL, ci.dataType, ci.length }, ColumnMigration::Action::DELETED });
            if (!inserted.second) {
                if (inserted.first->definition.db_size != ci.length && orm.session.get_backend_name() != "sqlite3")
                    const_cast<ColumnMigration::Action&>(inserted.first->action) = ColumnMigration::Action::ALTERED;
                else
                    migration.erase(inserted.first);
            }
        }
    }

    // Perform migration
    for (auto& migration_info : migration) {
        auto& def = migration_info.definition;
        switch (migration_info.action) {
            case ColumnMigration::Action::CREATED: orm.session.add_column(table, def.name, def.db_type, def.db_size); break;
            case ColumnMigration::Action::DELETED: orm.session.drop_column(table, def.name); break;
            case ColumnMigration::Action::ALTERED: orm.session.alter_column(table, def.name, def.db_type, def.db_size); break;
            default: break;
        }
    }

    if (migration.empty())
        result.erase(table);

#define SOCI_ORM_COLLECTION(FIELD)                                                                  \
    {                                                                                               \
        typedef std::decay_t<SOCI_ORM_ACCESSOR::FIELD::value_type> ValueType;                       \
        auto sub = soci_orm::ORM<ValueType>::migrate(orm);                                          \
        result.insert(std::make_move_iterator(sub.begin()), std::make_move_iterator(sub.end()));    \
    }
#include "soci_xmacro.h"

    return result;
}