#define SOCI_ORM_FK(CLASS)                                                                                                  \
template<> const std::string& soci_orm::ORM<CLASS>::details ::get_table();                                                  \
template<> const soci_orm::utils::ColumnsDefinition& soci_orm::ORM<CLASS>::details::get_columns_definition();               \
template<> int soci_orm::Single<CLASS>::compare(const soci_orm::Single<CLASS>& other) const;                                \
template<> void soci_orm::Bulk<CLASS>::PrimaryKey::resize(size_t size);                                                     \
template<> template<> void soci_orm::Bulk<CLASS>::PrimaryKey::append<CLASS>(const CLASS& value, int64_t index);             \
template<> void soci_orm::Bulk<CLASS>::PrimaryKey::get(soci_orm::Single<CLASS>& value, size_t index);                       \
template<> void soci_orm::Bulk<CLASS>::PrimaryKey::into(soci::statement& stmt, std::vector<soci::indicator>& indicators);   \
template<> void soci_orm::Bulk<CLASS>::PrimaryKey::use(soci::statement& stmt) const;                                        \

#include "soci_xmacro.h"

template<> const std::string& SOCI_ORM_DETAILS::get_table();
template<> const soci_orm::utils::ColumnsDefinition& SOCI_ORM_DETAILS::get_columns_definition();
