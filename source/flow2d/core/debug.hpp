// @date 2016/06/07
// @author Mao Jingkai(oammix@gmail.com)

#include <flow2d/forward.hpp>
#include <cstdarg>

NS_FLOW2D_BEGIN

enum class LogLevel
{
    VERBOSE,
    INFORMATION,
    WARNING,
    ERROR,
};

struct Debug
{
    static void log(LogLevel, const char* msg, va_list args);
    static void log(LogLevel, const char* msg, ...);
    static void set_log_filter(LogLevel);

    // aborts the program execution and trys print the stacktrace
    static void abort(const char* file, int line, const char* format, ...);
    static void traceback();
};

#define FLOW_LOGIV(msg, va_list) Debug::log(LogLevel::INFORMATION, msg, va_list)
#define FLOW_LOGWV(msg, va_list) Debug::log(LogLevel::WARNING, msg, va_list)
#define FLOW_LOGEV(msg, va_list) Debug::log(LogLevel::ERROR, msg, va_list)

#define FLOW_LOGI(msg, ...) Debug::log(LogLevel::INFORMATION, msg, ##__VA_ARGS__)
#define FLOW_LOGW(msg, ...) Debug::log(LogLevel::WARNING, msg, ##__VA_ARGS__)
#define FLOW_LOGE(msg, ...) Debug::log(LogLevel::ERROR, msg, ##__VA_ARGS__)

#define FLOW_ASSERT(cond, msg, ...) do { \
    if(!(cond)) { \
        Debug::abort(__FILE__, __LINE__, "\nAssertion failed: %s, " msg "\n", #cond, ##__VA_ARGS__); \
    } \
} while(0)

#define FLOW_ASSERT_NOT_NULL(pointer) FLOW_ASSERT(pointer != nullptr, #pointer " must be not null")
#define FLOW_FATAL(msg) FLOW_ASSERT(false, msg)
#define FLOW_ENSURE(cond) FLOW_ASSERT(cond, "")

NS_FLOW2D_END