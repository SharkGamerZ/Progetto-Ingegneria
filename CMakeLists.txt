cmake_minimum_required(VERSION 3.10)

project (ing CXX)

set(CMAKE_CXX_STANDARD 23)

set(SOURCE_FILES
    src/main.cpp
    src/server/Customer.cpp
    src/server/Shipper.cpp
    src/server/Shipper.hpp
    src/server/Supplier.cpp
    src/dbutils/pgutils.cpp
    src/dbutils/rdutils.cpp
    src/test-generator/generator.cpp
)

add_executable(ing ${SOURCE_FILES})
target_link_libraries(ing PRIVATE -lhiredis -lpqxx -lpq)
