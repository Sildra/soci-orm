
/// @brief The accessor static class of the ORM
///
/// @details Contains static accessors to the main class and a repesentation of its layout.
///    Non instanciable.
template<>
struct SOCI_ORM_ACCESSOR
{
    accessor() = delete;
#define SOCI_ORM_VALUE_FULL(CONVERTOR, FIELD, NAME)                         \
    using FIELD = std::decay_t<decltype(SOCI_ORM_CLASS::FIELD)>;            \
    static const FIELD& get_ ## FIELD(const SOCI_ORM_CLASS& val)            \
    { return val.FIELD; }                                                   \
    static FIELD& get_ ## FIELD(SOCI_ORM_CLASS& val)                        \
    { return val.FIELD; }                                                   \
    static const FIELD& const_get_ ## FIELD(const SOCI_ORM_CLASS& val)      \
    { return val.FIELD; }                                                   \
    static FIELD& mutable_get_ ## FIELD(SOCI_ORM_CLASS& val)                \
    { return val.FIELD; }
#define SOCI_ORM_PK_FULL            SOCI_ORM_VALUE_FULL
#define SOCI_ORM_INLINE(FIELD)      SOCI_ORM_VALUE(FIELD, "")
#define SOCI_ORM_COLLECTION(FIELD)  SOCI_ORM_VALUE(FIELD, "")
#include "soci_xmacro.h"
};
