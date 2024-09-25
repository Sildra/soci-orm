This is an header-only library wrapper of the SOCI database library enabling the generation of tedious database code with a set of expansion-macro.

# Macro definition

To define a table, we use a code generator triggered by an include during the preprocessor compilation phase.
It is possible to generate multiple tables definition in the same c++ translation unit as each definition is a separate specialized template.

## Macro list


| Macro      | Parameter | Description | Requirements |
|------------|-------------|---|----|
| SOCI_ORM_TABLE | Table name | Associates the class to a table | Optional if the type is not associated to a table |
| SOCI_ORM_VALUE | Class member, Column name | Associates the member to a standard column |
| SOCI_ORM_INLINE | Class member | Inline the columns of the member to the current table |
| SOCI_ORM_PK | Class member, Column name | Associates the member to a standard column |
| SOCI_ORM_FK | Class name | Associates the class as a parent, using the parent Primary Key as a Foreign Key | Unique, the class cannot be loaded or saved by itself and will requires the parent |
| SOCI_ORM_COLLECTION | Class member | Associates the member as a child | The child must set the parent as a Foreign Key |
| SOCI_ORM_AUTOINDEX | Field name, Column name |  |  |
| SOCI_ORM_ACTION | New action, Update action |  |  |

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
    SOCI_ORM_FK(Data)               \
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

* [DONE] Automatic database migration
* [TODO] Bulk insertions to database
* [TODO] Insert/Update/Merge styles of operations
* [TODO] Cascading Deletes
* [TODO] Cascading Loading operations
* [TODO] Automatic Primary keys and default indexes
* [TODO] Built-in json integration with metaprogrammation
* [TODO] Customizable Indexes
* [TODO] Built-in audit tables for the ORM
