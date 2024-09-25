#include <vector>
#include <string>
#include <set>
#include <functional>

#include <soci/soci.h>

#include "utils.h"

namespace soci_orm {
    struct Orm {
        Orm(soci::session&& session) : session(std::move(session)) { }
        soci::session session;
    };

    /// @brief P-IMPL of the Repository
    /// @description Contains the information related to the state of the database for this specific T type
    /// @tparam T Specialization type of the Repository
    template<typename T>
    class Repository {
    public:
        struct Value;
        Repository();
        Repository(Repository<T>&& other);

        Value& find(const T& value, int64_t index);
    private:
        struct Key;
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };




    /// @brief P-IMPL of the ORM Saver
    /// @tparam T Specialization type of the Saver
    template<typename T>
    class Saver {
    public:
        class PrimaryKey {
        public:
            PrimaryKey();

            template<typename U>
            void append(const U& value, int64_t index);
            void exchange(soci::statement& stmt) const;
        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl;
        };

        class Data {
        public:
            Data();

            template<typename U>
            void append(const U& value, int64_t index);
            void append_action(utils::Action* action);
            void exchange(soci::statement& stmt) const;
            void execute(soci::statement& stmt, utils::Action action) const;
        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl;
        };

        Saver();
        ~Saver();
        Saver(Saver<T>&& other);
        Saver<T>& operator=(Saver<T>&& other);

        /// @brief Prepare a new value in the Saver
        /// @param value The value that will be added
        /// @param index The current index of the value if part of an inner collection
        /// @return The prepared value, where the potential foreign key can be added
        Data* append(Repository<T>* repository, const T* value, int64_t index)
        { return (value ? append(repository, *value, index) : nullptr); }
        /// @brief Prepare a new value in the Saver
        /// @param value The value that will be added
        /// @param index The current index of the value if part of an inner collection
        /// @return The prepared value, where the potential foreign key can be added
        Data* append(Repository<T>* repository, const T& value, int64_t index);
        /// @brief Save the prepared values in the database
        /// @param orm The database orm engine where the query will be executed
        /// @return The Audit of the loaded values segregated by tables
        utils::AuditTransaction save(Orm& orm);

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
        utils::AuditRows affected;
    };

    /// @brief P-IMPL of the ORM Loader
    /// @tparam T Specialization type of the Loader
    template<typename T>
    class Loader {
    public:
        struct ForeignKey {
            struct Impl;
            ForeignKey();
            ForeignKey(const Impl& value);
            ~ForeignKey();
            ForeignKey(ForeignKey&& other);
            ForeignKey& operator=(ForeignKey&& other);
            bool operator<(const ForeignKey& other) const;
            
            template<typename U>
            void fetch(typename Loader<U>::PrimaryKey*& value) const;
        private:
            std::unique_ptr<Impl> pimpl;
        };

        struct PrimaryKey {
            struct Impl;
            PrimaryKey();
            PrimaryKey(const T& value);
            ~PrimaryKey();
            PrimaryKey(PrimaryKey&& other);
            PrimaryKey& operator=(PrimaryKey&& other);
            bool operator<(const PrimaryKey& other) const;

            void copy_from(const PrimaryKey& other);
            void from_base(const soci::values& values, soci::indicator ind);
            void to_base(soci::values& values, soci::indicator ind) const;
        private:
            std::unique_ptr<Impl> pimpl;
        };
        struct Mapper;

        Loader(Orm& orm, const std::string& filter);
        ~Loader();
        Loader(Loader<T>&& other);
        Loader<T>& operator=(Loader<T>&& other);

        /// @brief Fetch a single value from the Loader and advance internal iterators
        /// @return A value of type T or empty once the iterators reached the end
        std::unique_ptr<T> fetch();

        /// @brief Fetch a single value from the Loader and advance internal PK iterator
        /// @return A value of type T or empty once the PK iterator reached the end
        std::unique_ptr<T> fetch_pk();

        /// @brief Fetch the foreign key from the Loader and advance internal FK iterator
        /// @return A pointer to the ForeignKey or nullptr if the PK iterator reached the end
        const ForeignKey* fetch_fk();

        /// @brief Peek the foreign key from the Loader
        /// @return A pointer to the ForeignKey or nullptr if the PK iterator reached the end
        const ForeignKey* peek_fk();

        /// @brief Get the TransactionResults of the loader
        /// @return A structure containing the description of the loaded values and encountered loading errors
        const utils::AuditTransaction& get_transaction_result() const;

        /// @brief Create a new loader and execute the queries against the database
        /// @param orm The database orm engine where the query will be executed
        /// @param filter A filter subquery
        /// @return A Loader containing the loaded values, ready to be fetched
        static std::unique_ptr<Loader<T>> make_loader(Orm& orm, const std::string& filter);

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

    template<typename T>
    struct ORM {
        ORM() = delete;
        struct accessor;
        /// @brief The details of the ORM interface, available to perform fine-grained operations
        struct details {
            details() = delete;
            /// @brief Returns the table name associated to the current ORM
            /// @return A string containing the table name
            static const std::string& get_table();
            static void fetch_pk(std::vector<std::string>& pks);
            static void fetch_fields(std::vector<std::string>& fields);
            static void fetch_fk(std::vector<std::string>& fks);

            static inline std::vector<std::string> get_pk()
            { std::vector<std::string> v; fetch_pk(v); return v; }
            static inline std::vector<std::string> get_fk()
            { std::vector<std::string> v; fetch_fk(v); return v; }
            static inline std::vector<std::string> get_fields()
            { std::vector<std::string> v; fetch_fields(v); return v; }

            // Migration utilities
            static void create_columns(Orm& orm, const std::string& original_table,
                utils::AuditMigrationTable& audit);
            static void remove_columns(Orm& orm, const std::string& original_table,
                utils::AuditMigrationTable& audit);
            static void add_pk(soci::ddl_type& ddl);
            static void add_pk_constraints(soci::ddl_type& ddl);
            static void add_fk_constraints(soci::ddl_type& ddl);
            static void add_index(Orm& orm, const std::string& name, const std::vector<std::string>& columns);

            static void migrate(soci::ddl_type& ddl, const std::string& original_table, std::set<std::string>& current_columns);

            // Statement builders
            static std::string build_insert_statement();
            static std::string build_update_statement();
            static std::string build_merge_statement_std(soci::session& sql);
            static std::string build_merge_statement_sqlite();

            static inline const std::string& insert_statement()
            { static std::string stmt = build_insert_statement(); return stmt; }
            static inline const std::string& update_statement()
            { static std::string stmt = build_update_statement(); return stmt; }
            static inline const std::string& merge_statement_std(soci::session& sql)
            { static std::string stmt = build_merge_statement_std(sql); return stmt; }
            static inline const std::string& merge_statement_sqlite(soci::session&)
            { static std::string stmt = build_merge_statement_sqlite(); return stmt; }
            static inline const std::string& merge_statement_throw(soci::session&)
            { throw std::logic_error("Unhandled dataabase type for merge"); }

            static inline const std::string& merge_statement(soci::session& sql) {
                using merge_fct = std::function<const std::string&(soci::session&)>;
                static std::map<std::string, merge_fct> merger = {
                    { "sqlite3", merge_fct(merge_statement_sqlite) },
                    { "oracle", merge_fct(merge_statement_std) },
                };
                auto lookup = std::make_pair(sql.get_backend_name(), merge_fct(merge_statement_throw));
                return merger.insert(lookup).first->second(sql);
            }


            static inline soci::statement prepare_select(soci::session& sql, const std::string& filter)
            { return (sql.prepare << "select * from " << get_table() << filter); }
            static inline soci::statement prepare_insert(soci::session& sql)
            { return (sql.prepare << insert_statement()); }
            static inline soci::statement prepare_update(soci::session& sql)
            { return (sql.prepare << update_statement()); }
            static inline soci::statement prepare_merge(soci::session& sql)
            { return (sql.prepare << merge_statement(sql)); }
        };

        /// @brief Migrate the schema for the current type and its children
        /// @param orm The database orm engine where the query will be executed
        /// @return The Audit of the migration performed on the database
        static soci_orm::utils::AuditMigrationTables migrate(Orm& orm);

        /// @brief Create a loader for the type T and fetch the data from the database
        /// @param orm The database orm engine where the query will be executed
        /// @param filter A filter subquery
        /// @return A Loader containing the loaded values, ready to be fetched
        static inline Loader<T> create_loader(Orm& orm, const std::string& filter)
        {
            return soci_orm::Loader<T>(orm, filter);
        }

        /// @brief Fetch the data from the database and load them in the collection of type C
        /// @tparam C The type of the collection
        /// @param orm The database orm engine where the query will be executed
        /// @param filter A filter subquery
        /// @param collection The collection where the data will be loaded
        /// @return The Audit of the loaded values segregated by tables
        template<typename C>
        static inline utils::AuditTransaction load(Orm& orm, const std::string& filter, C& collection)
        {
            auto loader = create_loader(orm, filter);
            auto inserter = std::inserter(collection, collection.end());
            while (auto data = loader->fetch()) {
                utils::smart_inserter<decltype(inserter)>::insert(inserter, data);
            }
            return loader->get_transaction_result();
        }

        static inline Saver<T> create_saver()
        {
            return Saver<T>();
        }

        static inline Saver<T> create_saver(Repository<T>* repository, const T& value)
        {
            Saver<T> saver;
            saver.append(repository, value, 0);
            return saver;
        }

        template<typename Iter>
        static inline Saver<T> create_saver(Repository<T>* repository,Iter it, Iter end)
        {
            Saver<T> saver;
            int64_t index = -1;
            for (; it != end; ++it)
                saver.append(repository, *it, ++index);
            return saver;
        }

        static utils::AuditTransaction save(Orm& orm, Repository<T>* repository, const T& value) {
            return create_saver(repository, value).save(orm);
        }

        template<typename Iter>
        static utils::AuditTransaction save(Orm& orm, Repository<T>* repository, Iter it, Iter end) {
            return create_saver(repository, it, end).save(orm);
        }
        
        template<typename Coll>
        static utils::AuditTransaction save(Orm& orm, Repository<T>* repository, const Coll& coll) {
            return create_saver(repository, coll.begin(), coll.end()).save(orm);
        }
    };
} /* !namespace soci_orm */
