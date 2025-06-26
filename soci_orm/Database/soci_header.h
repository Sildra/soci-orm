#pragma once

#include <vector>
#include <string>
#include <set>
#include <functional>
#include <chrono>

#include <soci/soci.h>

#include "utils.h"

namespace soci_orm {
    struct Orm {
        using chrono = std::chrono::high_resolution_clock;
        Orm(soci::session&& session) : session(std::move(session)) { }
        soci::session session;
        size_t fetch_size { 1000 };
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

    template<typename T> class Bulk;
    template<typename T>
    class Single {
        friend class Bulk<T>;
    public:
        Single();
        ~Single();
        Single(Single<T>&&) noexcept;

        template<typename U>
        const Single<U>& get() const;

        int compare(const Single<T>& other) const;
        bool operator<(const Single<T>& other) const { return compare(other) < 0; }

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

    /// @brief A Bulk object represented as a vectorized version of a class, suitable for bulk insertions in databases
    /// @tparam T The class that will serve as the layout of the data
    template<typename T>
    class Bulk {
    public:
        /// @brief The PrimaryKey part of a Bulk object
        class PrimaryKey {
            friend class Bulk<T>;
        public:
            /// @brief Constructor
            PrimaryKey();
            ~PrimaryKey();

            void resize(size_t size);

            template<typename U>
            void append(const U& value, int64_t index);
            void get(T& value, size_t index);
            void get(Single<T>& value, size_t index);
            void into(soci::statement& stmt, std::vector<soci::indicator>& indicators);
            void use(soci::statement& stmt) const;
        private:
            struct Impl;
            std::unique_ptr<Impl> pimpl;
        };
        /// @brief Constructor
        Bulk();
        ~Bulk();

        /// @brief Resize the Bulk buffers
        /// @param size The size of the buffers
        void resize(size_t size);

        /// @brief Append a class to the Bulk object
        /// @tparam U The class to append
        /// @param value The value to append
        /// @param index The current autoindex value
        template<typename U>
        void append(const U& value, int64_t index);

        /// @brief Append a reference of the repository action to the Bulk object
        /// @param action The potentially null reference to the action that will be set
        void append_action(utils::Action* action);

        void get(T& value, size_t index);
        void get(Single<T>& value, size_t index);

        /// @brief Associate into buffers for database load
        /// @param stmt The statement where the perparation occurs
        /// @param indicators The potentially null indicators
        void into(soci::statement& stmt, std::vector<soci::indicator>& indicators);

        /// @brief Associate use buffers for database save
        /// @param stmt The statement where the perparation occurs
        void use(soci::statement& stmt) const;

        /// @brief Save the bulk request that has been previously constructed
        /// @param stmt The statement to execute
        /// @param action The action to set in the repository once executed
        void save(soci::statement& stmt, utils::Action action) const;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

    /// @brief P-IMPL of the ORM Saver
    /// @tparam T Specialization type of the Saver
    template<typename T>
    class Saver {
    public:
        Saver();
        ~Saver();
        Saver(Saver<T>&& other);
        Saver<T>& operator=(Saver<T>&& other);

        /// @brief Prepare a new value in the Saver
        /// @param value The value that will be added
        /// @param index The current index of the value if part of an inner collection
        /// @return The prepared value, where the potential foreign key can be added
        Bulk<T>* append(Repository<T>* repository, const T* value, int64_t index)
        { return (value ? append(repository, *value, index) : nullptr); }
        /// @brief Prepare a new value in the Saver
        /// @param value The value that will be added
        /// @param index The current index of the value if part of an inner collection
        /// @return The prepared value, where the potential foreign key can be added
        Bulk<T>* append(Repository<T>* repository, const T& value, int64_t index);
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
        Loader(Orm& orm, const std::string& filter);
        ~Loader();
        Loader(Loader<T>&& other);
        Loader<T>& operator=(Loader<T>&& other);
        
        /// @brief Fetch the next value contained in the Loader
        /// @return A pointer containing the next value or null if the Loader reached the end
        std::unique_ptr<T> fetch();

        /// @brief Peek the next PrimaryKey of the Loader
        /// @return 
        const Single<T>& peek_pk() const;

        /// @brief Check if there is still data in the Loader
        bool end() const;

        /// @brief Inrcements the Loader
        /// @warning Does not check for out-of-bound access
        void next();


        /// @brief Get the TransactionResults of the loader
        /// @return A structure containing the description of the loaded values and encountered loading errors
        const utils::AuditTransaction& get_transaction_result() const;

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
            /// @brief Get the computed column definition associated to the current ORM
            /// @return A vector containing the columns definition ordered by the following types { FK, PK, AUTOINDEX, COL }, names order is preserved
            static const utils::ColumnsDefinition& get_columns_definition();

            // Migration utilities
            static void add_index(Orm& orm, const std::string& name, const std::vector<std::string>& columns);

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
            { throw std::logic_error("Unhandled database type for merge"); }

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
        static soci_orm::utils::TablesMigration migrate(Orm& orm);

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
            while (auto data = loader.fetch()) {
                utils::smart_inserter<decltype(inserter)>::insert(inserter, data);
            }
            return loader.get_transaction_result();
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
        static inline Saver<T> create_saver(Repository<T>* repository, Iter it, Iter end)
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
