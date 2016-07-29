#pragma once

#include <precompile.hpp>

#define NS_FLOW2D_BEGIN namespace flow2d {
#define NS_FLOW2D_END }

NS_FLOW2D_BEGIN

#define INLINE inline

/// FORWARD DECLARATIONS
extern void ABORT(const char* file, int line, const char* format, ...);
extern void LOGI(const char* format, ...);
extern void LOGW(const char* format, ...);
extern void LOGE(const char* format, ...);

#define ASSERT(condition, format, ...) do { \
    if( !(condition) ) { \
        ABORT(__FILE__, __LINE__, "\nAssertion failed: %s, " format "", #condition, ##__VA_ARGS__);\
    } \
} while(0)

#define FATAL(format, ...) ASSERT(false, format, ##__VA_ARGS__)
#define ENSURE(condition) ASSERT(condition, "")

#define NOTUSED(v) for(;;) { (void)(1 ? (void)0 : ((void)v)); break; }

NS_FLOW2D_END