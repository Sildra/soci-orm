#ifndef SOCI_ORM_CLASS
# error "SOCI_ORM_CLASS macro must define the generated class"
#else

#include "private/affinity.h"

#include "private/generator/header.h"
#define SOCI_ORM_ACCESSOR soci_orm::ORM<SOCI_ORM_CLASS>::accessor

#include "private/generator/accessor.hxx"
#include "private/generator/repository.hxx"
#include "private/generator/saver.hxx"
#include "private/generator/loader.hxx"
#include "private/generator/details.hxx"
#include "private/generator/class.hxx"

#undef SOCI_ORM_CLASS
#undef SOCI_ORM_MACRO
#undef SOCI_ORM_ACCESSOR

#endif /* SOCI_ORM_CLASS */
