cmake_minimum_required(VERSION 3.10)
project(mcpmod)

find_package(pybind11 REQUIRED)
message(STATUS "pybind11 found: ${pybind11_INCLUDE_DIRS}")

# Encontrar Eigen
find_package(Eigen3 REQUIRED)
message(STATUS "Eigen3 found: ${EIGEN3_INCLUDE_DIR}")

# Buscar redis++ library
find_library(REDISPP_LIBRARY NAMES redis++ HINTS /usr/local/lib /usr/lib)
if(NOT REDISPP_LIBRARY)
    message(FATAL_ERROR "redis++ no encontrado. Instala redis-plus-plus.")
endif()
message(STATUS "redis++ library: ${REDISPP_LIBRARY}")

# Buscar hiredis
find_library(HIREDIS_LIBRARY NAMES hiredis HINTS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
if(NOT HIREDIS_LIBRARY)
    message(FATAL_ERROR "hiredis no encontrado. Instala libhiredis-dev.")
endif()
message(STATUS "hiredis found: ${HIREDIS_LIBRARY}")

# Buscar redis++ headers
find_path(REDISPP_INCLUDE_DIR NAMES sw/redis++/redis++.h HINTS /usr/local/include /usr/include)
if(NOT REDISPP_INCLUDE_DIR)
    message(FATAL_ERROR "redis++ headers no encontrados. Instala redis-plus-plus.")
endif()
message(STATUS "redis++ include dir: ${REDISPP_INCLUDE_DIR}")

add_library(mcp_module MODULE bindings.cpp)
target_link_libraries(mcp_module PRIVATE 
    pybind11::module 
    ${REDISPP_LIBRARY} 
    ${HIREDIS_LIBRARY}
    Eigen3::Eigen
)
set_target_properties(mcp_module PROPERTIES 
    PREFIX "" 
    SUFFIX ".so"
    LINK_FLAGS "-Wl,-rpath,/usr/local/lib"
)

# Incluir directorios
include_directories(${pybind11_INCLUDE_DIRS} ${EIGEN3_INCLUDE_DIR} ${REDISPP_INCLUDE_DIR})