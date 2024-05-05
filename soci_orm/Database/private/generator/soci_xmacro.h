#ifndef SOCI_ORM_TABLE
# define SOCI_ORM_TABLE(NAME)
#endif
#ifndef SOCI_ORM_VALUE
# define SOCI_ORM_VALUE(FIELD, NAME)
#endif
#ifndef SOCI_ORM_INLINE
# define SOCI_ORM_INLINE(FIELD)
#endif
#ifndef SOCI_ORM_PK
# define SOCI_ORM_PK(FIELD, NAME)
#endif
#ifndef SOCI_ORM_FK
# define SOCI_ORM_FK(CLASS)
#endif
#ifndef SOCI_ORM_COLLECTION
# define SOCI_ORM_COLLECTION(FIELD)
#endif
#ifndef SOCI_ORM_AUTOINDEX
# define SOCI_ORM_AUTOINDEX(FIELD, NAME)
#endif
#ifndef SOCI_ORM_ACTION_TRANSITION
# define SOCI_ORM_ACTION_TRANSITION(STORAGE, ACTION_AFTER_SYNC)
#endif

SOCI_ORM_MACRO

#undef SOCI_ORM_TABLE
#undef SOCI_ORM_VALUE
#undef SOCI_ORM_INLINE
#undef SOCI_ORM_PK
#undef SOCI_ORM_FK
#undef SOCI_ORM_COLLECTION
#undef SOCI_ORM_AUTOINDEX
#undef SOCI_ORM_ACTION_TRANSITION