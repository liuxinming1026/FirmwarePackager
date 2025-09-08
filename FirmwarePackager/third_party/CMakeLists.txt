# Third-party dependencies

# nlohmann json library
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/nlohmann
)

# GoogleTest for unit testing
add_subdirectory(googletest-1.17.0 EXCLUDE_FROM_ALL)

# Prebuilt libraries
add_library(LibArchive::LibArchive SHARED IMPORTED GLOBAL)
set_target_properties(LibArchive::LibArchive PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/libarchive/lib/archive.lib"  # Windows path
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/libarchive/include"
)

add_library(OpenSSL::SSL SHARED IMPORTED)
set_target_properties(OpenSSL::SSL PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/openssl/lib/libssl.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/openssl/include"
)

add_library(OpenSSL::Crypto SHARED IMPORTED)
set_target_properties(OpenSSL::Crypto PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/openssl/lib/libcrypto.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/openssl/include"
)

