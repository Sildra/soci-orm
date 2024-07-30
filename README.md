This is an header-only library wrapper of the SOCI database library enabling the generation of tedious database code with a set of expansion-macro.

```c++
// data.h
struct Inner {
    std::string value;
};

struct Data {
    int value;
    std::vector<Inner> inner;
};

// data.cpp
#define SOCI_ORM_MACRO              \
    SOCI_ORM_TABLE("INNER")         \
    SOCI_ORM_VALUE(value, "VALUE")  \

#define SOCI_ORM_CLASS Inner
#include "Database/soci_source_generate.hxx"

#define SOCI_ORM_MACRO              \
    SOCI_ORM_TABLE("DATA")          \
    SOCI_ORM_VALUE(value, "VALUE")  \
    SOCI_ORM_COLLECTION(inner)      \

#define SOCI_ORM_CLASS Data
#include "Database/soci_source_generate.hxx"
```

```txt
# Database layout:
+================+             +===============+
|      DATA      |             |     INNER     |
+----------------+ ----------> +---------------+
| VALUE: int     |             | VALUE: string |
+================+             +===============+

```

Planned features :

[DONE] Automatic database migration
[TODO] Bulk insertions to database
[TODO] Insert/Update/Merge styles of operations
[TODO] Cascading Deletes
[TODO] Cascading Loading operations
[TODO] Automatic Primary keys and default indexes
[TODO] Built-in json integration with metaprogrammation
[TODO] Customizable Indexes
[TODO] Built-in audit tables for the ORM
