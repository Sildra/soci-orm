#include <vector>
#include <string>
#include <set>

#include <soci/soci.h>

#include "utils.h"

namespace soci_orm {
    struct Orm {
        Orm(soci::session&& session) : session(session) { }
        soci::session session;
    };

    /// @brief P-IMPL of the Repository
    /// @description Contains the information related to the state of the database for this specific T type
    /// @tparam T Specialization type of the Repository
    template<typename T>
    class Repository final {
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

    template<typename T>
    class SaverData final {
    public:
        SaverData();
        ~SaverData();

        template<typename U>
        void append(const U& value, index);
        void exchange() const;
    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

    /// @brief P-IMPL of the ORM Saver
    /// @tparam T Specialization type of the Saver
    template<typename T>
    class Saver final {
    public:
        /// @brief Prepare a new value in the Saver
        /// @param value The value that will be added
        /// @param index The current index of the value if part of an inner collection
        /// @return The prepared value, where the potential foreign key can be added
        SaverData<T>* append(Repository<T>* repository, const T* value, int64_t index)
        { return (value ? append(repository, *value, index) : nullptr); }
        /// @brief Prepare a new value in the Saver
        /// @param value The value that will be added
        /// @param index The current index of the value if part of an inner collection
        /// @return The prepared value, where the potential foreign key can be added
        SaverData<T>* append(Repository<T>* repository, const T& value, int64_t index) = 0;
        /// @brief Save the prepared values in the database
        /// @param orm The database orm engine where the query will be executed
        /// @return The Audit of the loaded values segregated by tables
        utils::AuditTransaction save(Orm& orm);

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
    };

    /// @brief P-IMPL of the ORM Loader
    /// @tparam T Specialization type of the Loader
    template<typename T>
    class Loader final {
    public:
        /// @brief Fetch a single value from the Loader and advance internal iterators
        /// @return A value of type T or empty once the iterators reached the end
        std::unique_ptr<T> fetch();

        /// @brief Get the TransactionResults of the loader
        /// @return A structure containing the description of the loaded values and encountered loading errors
        const utils::AuditTransaction& get_transaction_result() const { return transaction_result; }

        /// @brief Create a new loader and execute the queries against the database
        /// @param orm The database orm engine where the query will be executed
        /// @param filter A filter subquery
        /// @return A Loader containing the loaded values, ready to be fetched
        static std::unique_ptr<Loader<T>> make_loader(Orm& orm, const std::string& filter);

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl;
        utils::AuditTransaction transaction_result;
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
        };

        /// @brief Migrate the schema for the current type and its children
        /// @param orm The database orm engine where the query will be executed
        /// @return The Audit of the migration performed on the database
        static soci_orm::utils::AuditMigrationTables migrate(Orm& orm);

#if 0
        /// @brief Create a loader for the type T and fetch the data from the database
        /// @param orm The database orm engine where the query will be executed
        /// @param filter A filter subquery
        /// @return A Loader containing the loaded values, ready to be fetched
        static inline std::unique_ptr<Loader<T>> create_loader(Orm& orm, const std::string& filter)
        {
            return soci_orm::Loader<T>::make_loader(orm, filter);
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

        static std::unique_ptr<Saver<T>> create_saver();

        static inline std::unique_ptr<Saver<T>> create_saver(const T& value)
        {
            auto saver = create_saver();
            saver->append(value, 0);
            return saver;
        }

        template<typename Iter>
        static inline std::unique_ptr<Saver<T>> create_saver(Iter it, Iter end)
        {
            auto saver = create_saver();
            int64_t index = -1;
            for (; it != end; ++it)
                saver->append(*it, ++index);
            return saver;
        }

        static utils::AuditTransaction save(Orm& orm, const T& value) {
            return create_saver(value)->save(orm);
        }

        template<typename Iter>
        static utils::AuditTransaction save(Orm& orm, Iter it, Iter end) {
            return create_saver(it, end)->save(orm);
        }
        
        template<typename Coll>
        static utils::AuditTransaction save(Orm& orm, const Coll& end) {
            return create_saver(it, end)->save(orm);
        }
#endif
    };
} /* !namespace soci_orm */
