#ifndef SOCI_ORM_CLASS
# error "SOCI_ORM_CLASS macro must define the generated class"
#else

#include "soci_header.h"

#include "private/affinity.h"

#define SOCI_ORM_MAIN       soci_orm::ORM<SOCI_ORM_CLASS>
#define SOCI_ORM_ACCESSOR   SOCI_ORM_MAIN::accessor
#define SOCI_ORM_DETAILS    SOCI_ORM_MAIN::details
#define SOCI_ORM_REPOSITORY soci_orm::Repository<SOCI_ORM_CLASS>
#define SOCI_ORM_SINGLE     soci_orm::Single<SOCI_ORM_CLASS>
#define SOCI_ORM_BULK       soci_orm::Bulk<SOCI_ORM_CLASS>
#define SOCI_ORM_SAVER      soci_orm::Saver<SOCI_ORM_CLASS>
#define SOCI_ORM_LOADER     soci_orm::Loader<SOCI_ORM_CLASS>

#include "private/generator/forward_declarations.hxx"
#include "private/generator/accessor.hxx"
#include "private/generator/repository.hxx"
#include "private/generator/data.hxx"
#include "private/generator/saver.hxx"
#include "private/generator/loader.hxx"
#include "private/generator/details.hxx"
#include "private/generator/class.hxx"

#undef SOCI_ORM_CLASS
#undef SOCI_ORM_MACRO
#undef SOCI_ORM_MAIN
#undef SOCI_ORM_DETAILS
#undef SOCI_ORM_ACCESSOR
#undef SOCI_ORM_REPOSITORY
#undef SOCI_ORM_SINGLE
#undef SOCI_ORM_BULK
#undef SOCI_ORM_SAVER
#undef SOCI_ORM_LOADER

#endif /* SOCI_ORM_CLASS */
