project (nome CXX)

set (CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES
        src/main.cpp
        src/server/customers.cpp
        src/server/fornitori.cpp
        src/server/produttori.cpp

        src/dbutils/pgutils.cpp
        src/dbutils/rdutils.cpp
        )

# Aggiungi tutte le sorgenti del progetto
add_executable(nome ${SOURCE_FILES})