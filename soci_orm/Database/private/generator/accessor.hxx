
namespace soci_orm {
/// @brief The accessor static class of the ORM
///
/// @details Contains static accessors to the main class and a repesentation of its layout.
///    Non instanciable.
template<>
struct SOCI_ORM_ACCESSOR
{
    accessor() = delete;
#define SOCI_ORM_VALUE(FIELD, NAME)                                                 \
    std::decay_t<decltype(SOCI_ORM_CLASS::FIELD)> FIELD;                            \
    static const decltype(FIELD)& get_ ## FIELD(const SOCI_ORM_CLASS& val)          \
    { return val.FIELD; }                                                           \
    static decltype(FIELD)& get_ ## FIELD(SOCI_ORM_CLASS& val)                      \
    { return val.FIELD; }                                                           \
    static const decltype(FIELD)& const_get_ ## FIELD(const SOCI_ORM_CLASS& val)    \
    { return val.FIELD; }                                                           \
    static decltype(FIELD)& mutable_get_ ## FIELD(SOCI_ORM_CLASS& val)              \
    { return val.FIELD; }
#define SOCI_ORM_PK                 SOCI_ORM_VALUE
#define SOCI_ORM_INLINE(FIELD)      SOCI_ORM_VALUE(FIELD, "")
#define SOCI_ORM_COLLECTION(FIELD)  SOCI_ORM_VALUE(FIELD, "")
#include "soci_xmacro.h"
};
} /* !namespace soci_orm */
