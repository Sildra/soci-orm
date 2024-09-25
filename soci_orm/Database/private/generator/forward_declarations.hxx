namespace soci_orm {
#define SOCI_ORM_FK(CLASS)                                                                              \
template<> void ORM<CLASS>::details::add_fk_constraints(soci::ddl_type& ddl);                           \
template<> void ORM<CLASS>::details::add_pk(soci::ddl_type& ddl);                                       \
template<> void ORM<CLASS>::details::fetch_pk(std::vector<std::string>& pks);                           \
template<> template<> void Saver<CLASS>::Data::append(const CLASS& value, int64_t index);               \
template<> void Saver<CLASS>::Data::exchange(soci::statement& stmt) const;                              \
template<> Loader<CLASS>::PrimaryKey::PrimaryKey();                                                     \
template<> Loader<CLASS>::PrimaryKey::~PrimaryKey();                                                    \
template<> void Loader<CLASS>::PrimaryKey::to_base(soci::values& values, soci::indicator ind) const;    \
template<> void Loader<CLASS>::PrimaryKey::copy_from(const PrimaryKey& other);                          \
template<> bool Loader<CLASS>::PrimaryKey::operator<(const PrimaryKey& other) const;                    \
template<> void Loader<CLASS>::PrimaryKey::from_base(const soci::values& values, soci::indicator ind); \

#include "soci_xmacro.h"

template<> const std::string& ORM<SOCI_ORM_CLASS>::details::get_table();
template<> Loader<SOCI_ORM_CLASS>::ForeignKey::ForeignKey();
template<> Loader<SOCI_ORM_CLASS>::ForeignKey::ForeignKey(const Impl& value);
template<> Loader<SOCI_ORM_CLASS>::ForeignKey::~ForeignKey();
template<> Loader<SOCI_ORM_CLASS>::PrimaryKey::PrimaryKey();
template<> Loader<SOCI_ORM_CLASS>::PrimaryKey::PrimaryKey(const SOCI_ORM_CLASS& value);
template<> Loader<SOCI_ORM_CLASS>::PrimaryKey::~PrimaryKey();
template<> bool Loader<SOCI_ORM_CLASS>::PrimaryKey::operator<(const PrimaryKey& other) const;


} /* !namespace soci_orm */