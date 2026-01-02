#pragma once

/**
 * @file Zyrnix_features.hpp
 * @brief Feature flags for conditional compilation
 * 
 * Use these flags to disable specific features and reduce binary size.
 * Define these macros before including Zyrnix headers or in your build system.
 * 
 * Example CMake usage:
 * @code
 * target_compile_definitions(my_app PRIVATE XLOG_NO_ASYNC XLOG_NO_JSON)
 * @endcode
 * 
 * Example compile command:
 * @code
 * g++ -DXLOG_NO_ASYNC -DXLOG_NO_JSON main.cpp -lZyrnix
 * @endcode
 */


#ifdef XLOG_MINIMAL
    #define XLOG_NO_ASYNC
    #define XLOG_NO_JSON
    #define XLOG_NO_NETWORK
    #define XLOG_NO_COLORS
    #define XLOG_NO_FILE_ROTATION
    #define XLOG_NO_CONTEXT
    #define XLOG_NO_FILTERS
#endif


#ifdef XLOG_NO_STRUCTURED
    #define XLOG_NO_JSON
#endif


#ifndef XLOG_NO_ASYNC
    #define XLOG_HAS_ASYNC 1
#else
    #define XLOG_HAS_ASYNC 0
#endif

#ifndef XLOG_NO_JSON
    #define XLOG_HAS_JSON 1
#else
    #define XLOG_HAS_JSON 0
#endif

#ifndef XLOG_NO_NETWORK
    #define XLOG_HAS_NETWORK 1
#else
    #define XLOG_HAS_NETWORK 0
#endif

#ifndef XLOG_NO_COLORS
    #define XLOG_HAS_COLORS 1
#else
    #define XLOG_HAS_COLORS 0
#endif

#ifndef XLOG_NO_FILE_ROTATION
    #define XLOG_HAS_FILE_ROTATION 1
#else
    #define XLOG_HAS_FILE_ROTATION 0
#endif

#ifndef XLOG_NO_CONTEXT
    #define XLOG_HAS_CONTEXT 1
#else
    #define XLOG_HAS_CONTEXT 0
#endif

#ifndef XLOG_NO_FILTERS
    #define XLOG_HAS_FILTERS 1
#else
    #define XLOG_HAS_FILTERS 0
#endif
