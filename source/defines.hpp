#pragma once

#include <precompile.hpp>

#define NS_FLOW2D_BEGIN namespace flow2d {
#define NS_FLOW2D_END }

#if defined(IOS)
#define PLATFORM_IOS
#elif defined(ANDROID)
#define PLATFORM_ANDROID
#elif defined(_WIN32)
#define PLATFORM_WIN32
#else
#define PLATFORM_UNIX
#endif

NS_FLOW2D_BEGIN

#define INLINE inline

/// FORWARD DECLARATIONS
extern void ABORT(const char* file, int line, const char* format, ...);
extern void LOGI(const char* format, ...);
extern void LOGW(const char* format, ...);
extern void LOGE(const char* format, ...);
extern void SET_DEBUG_CONFIG(int filter, bool exception);

const static int LOG_VERBOSE = 0;
const static int LOG_INFORMATION = 1;
const static int LOG_WARNING = 2;
const static int LOG_ERROR = 3;

#define ASSERT(condition, format, ...) do { \
    if( !(condition) ) { \
        ABORT(__FILE__, __LINE__, "\nAssertion failed: %s, " format "", #condition, ##__VA_ARGS__);\
    } \
} while(0)

#define FATAL(format, ...) ASSERT(false, format, ##__VA_ARGS__)
#define ENSURE(condition) ASSERT(condition, "")

///
#define NOTUSED(v) for(;;) { (void)(1 ? (void)0 : ((void)v)); break; }

NS_FLOW2D_END