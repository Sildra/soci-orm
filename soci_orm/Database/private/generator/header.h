#pragma once

namespace soci_orm {
    /// REPOSITORY

    template<typename T>
    struct RepositoryKey;
    template<typename T>
    struct RepositoryValue {
        RepositoryValue() = delete;
        utils::Action* get_action();
    };
    template<typename T>
    struct Repository {
        Repository() = delete;
        RepositoryValue<T>& find(const T& value, int64_t index);
        static std::unique_ptr<Repository<T>> make_unique();
    };
    
    /// SAVER
    #if 0
    template<typename T>
    struct SaverInterface {
    protected:
        ~SaverInterface() = default;
    public:
        virtual void append(const T& value, int64_t index) = 0;
    };

    struct SaverDispatcher
    {
    protected:
        ~SaverDispatcher() = default;
    public:
        template<typename T>
        void append_pk(const T& value, int64_t index) const
        {
            if (auto* p = dynamic_cast<SaverInterface<T>(this)>)
                p->append(value, index);
        }
    };

    template<typename T>
    struct SaverData : public SaverDispatcher, public SaverInterface<T>
    {
        static_assert(std::is_same<T, void>::value, "Repository template instanciation error: cannot instanciate generic template");

        void exchange(soci::statement& stmt) const;

        static std::unique_ptr<SaverData<T>> make_unique();
        static std::unique_ptr<SaverData<T>> make_unique_pk();
    };

    /// LOADER


    template<typename T> struct SaverImpl;
    template<typename T> struct LoaderImpl;
    template<typename T> struct SaverDataImpl;
    template<typename T> struct SaverPkImpl;
    template<typename T> struct SaverFkImpl;
    template<typename T> struct LoaderDataImpl;
    template<typename T> struct LoaderPkImpl;
    template<typename T> struct LoaderFkImpl;

    template<typename T>
    struct LoaderData
    {
        soci::LoaderFkImpl<T> fk;
        T value;
    };

    template<typename T>
    struct LoaderPk
    {
        virtual ~LoaderPk() = default;
        virtual void copy_from(const LoaderPk<T>& other) = 0;
        virtual void from_base(const soci::values& values, soci::indicator& ind) = 0;
        virtual void to_base(soci::values& values, soci::indicator& ind) const = 0;
        static std::unique_ptr<LoaderPk<T>> make_loader();
    };

    template<typename T>
    struct LoaderFk
    {
        virtual ~LoaderFk() = default;
        virtual void fetch(const LoaderPk<T>*& value) const = 0;
    };

    struct LoaderDataDispatcher
    {
        virtual ~LoaderDataDispatcher() = default;
        template<typename T>
        void fetch_fk(const LoaderPk<T>*& value) const
        {
            if (const auto* p = dynamic_cast<LoaderFk<T>*>(this))
                p->fetch(value);
        }
    };
    #endif
} /* !namespace soci_orm */