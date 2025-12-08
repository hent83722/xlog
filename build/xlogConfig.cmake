

include("${CMAKE_CURRENT_LIST_DIR}/../../../../lib/cmake/xlog/xlogConfigVersion.cmake")

add_library(xlog STATIC IMPORTED)
set_target_properties(xlog PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libxlog.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../include"
)
