namespace soci_orm {
template<>
struct RepositoryKey<SOCI_ORM_CLASS> {
#define SOCI_ORM_PK(FIELD, NAME)                        \
    Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::orm_type FIELD;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
    int64_t FIELD;
#include "soci_xmacro.h"

    RepositoryKey(const SOCI_ORM_CLASS& value, int64_t index) {
#define SOCI_ORM_PK(FIELD, NAME)                        \
        this.FIELD = Affinity<decltype(SOCI_ORM_ACCESSOR::FIELD)>::to_bulk(SOCI_ORM_ACCESSOR::get_ ## FIELD(value));
#define SOCI_ORM_AUTOINDEX(FIELD, NAME)                 \
        this.FIELD = index;
#include "soci_xmacro.h"
    }

    bool operator<(const RepositoryKey<SOCI_ORM_CLASS>& other) const {
        constexpr bool tie_field = true;
        return std::tie(                tie_field
#define SOCI_ORM_PK(FIELD, NAME)        , FIELD
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) , FIELD
#include "soci_xmacro.h"
        ) < std::tie(                   tie_field
#define SOCI_ORM_PK(FIELD, NAME)        , other.FIELD
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) , other.FIELD
#include "soci_xmacro.h"
        );
    }
};

template<>
struct RepositoryValue<SOCI_ORM_CLASS> {
    utils::Action action = {
#define SOCI_ORM_ACTION(INITIAL_ACTION, SAVING_ACTION)  \
        INITIAL_ACTION
#include "soci_xmacro.h"
    };
#define SOCI_ORM_COLLECTION(FIELD)                      \
    std::unique_ptr<Repository<decltype(SOCI_ORM_ACCESSOR::FIELD)>> FIELD = Repository<decltype(SOCI_ORM_ACCESSOR::FIELD)>::make_unique();
#include "soci_xmacro.h"
    utils::Action* get_action() { return &action; }
};

template<>
struct Repository<SOCI_ORM_CLASS> {
    std::map<RepositoryKey<SOCI_ORM_CLASS>, RepositoryValue<SOCI_ORM_CLASS>> repository;
    RepositoryValue<SOCI_ORM_CLASS>& find(const SOCI_ORM_CLASS& value, int64_t index) {
        return repository[RepositoryKey<SOCI_ORM_CLASS>(value, index)];
    }
    static std::unique_ptr<Repository<SOCI_ORM_CLASS>> make_unique();
};
std::unique_ptr<Repository<SOCI_ORM_CLASS>> Repository<SOCI_ORM_CLASS>::make_unique() {
    return std::make_unique<Repository<SOCI_ORM_CLASS>>();
}

} /* !namespace soci_orm */