gdb -batch -ex "info types ^soci_orm" test.exe | cut -d $'\t' -f 2- | grep '^soci_orm:' | grep -v 'json' | tr ';' ' ' | sed 's/^/ptype \0/' > _gdb.gdb
gdb -batch -x _gdb.gdb test.exe > _definitions.cpp
rm _gdb.gdb