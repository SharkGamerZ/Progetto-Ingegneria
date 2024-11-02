project (ing CXX)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES
    src/main.cpp
    src/server/Customer.cpp
    src/server/Shipper.cpp
    src/server/Supplier.cpp
    src/dbutils/pgutils.cpp
)

add_executable(ing ${SOURCE_FILES})
target_link_libraries(ing PRIVATE -lhiredis -lpqxx)
