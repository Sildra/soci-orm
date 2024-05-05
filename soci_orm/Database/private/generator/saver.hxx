template<>
struct SaverPkImpl<SOCI_ORM_CLASS> final : public SaverDataDispatcher, public SaverData<SOCI_ORM_CLASS>
#define SOCI_ORM_FK(CLASS) , public SaverData<CLASS>
#include "soci_xmacro.h"
{
#define SOCI_ORM_PK(FIELD, NAME)        \
    std::vector<> FIELD;
#define SOCI_ORM_FK(CLASS)              \
    std::unique_ptr<SaverData<CLASS>> CLASS ## _fk;
#define SOCI_ORM_AUTOINDEX(FIELD, NAME) \
    std::vector<int64_t> FIELD;
#include "soci_xmacro.h"
};