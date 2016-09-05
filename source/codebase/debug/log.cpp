#include <codebase/debug/log.hpp>
#include <codebase/debug/stacktrace.hpp>

#include <core/task.hpp>

NS_LEMON_BEGIN

const static char* kANSIReset   = "\x1b[0m";
const static char* kANSIYellow  = "\x1b[33m";
const static char* kANSIRed     = "\x1b[31m";

static const char* stt[] =
{
    kANSIReset,
    kANSIReset,
    kANSIYellow,
    kANSIRed,
};

struct Log
{
    void write(LogLevel level, const char* format, va_list args)
    {
        if( static_cast<uint8_t>(level) < static_cast<uint8_t>(filter) )
            return;

        char buf[512];
        int len = vsnprintf(buf, sizeof(buf), format, args);
        buf[len] = '\0';

        (*out) << stt[static_cast<uint8_t>(level)] << buf << kANSIReset << std::endl;
    }

    void write(LogLevel level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        write(level, format, args);
        va_end(args);
    }

    LogLevel filter = LogLevel::INFORMATION;
    std::ostream* out = &std::cout;
};

static Log s_log;

void set_output_stream(LogLevel filter, std::ostream* stream)
{
    if( stream == nullptr )
        return;

    s_log.filter = filter;
    s_log.out = stream;
}

void ABORT(const char* file, int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    s_log.write(LogLevel::ERROR, format, args);
    s_log.write(LogLevel::ERROR, "\n\tIn: %s:%d\n\nStacktrace:", file, line);
    stacktrace(*s_log.out, 2);
    va_end(args);
}

void LOGI(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    s_log.write(LogLevel::INFORMATION, format, args);
    va_end(args);
}

void LOGW(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    s_log.write(LogLevel::WARNING, format, args);
    va_end(args);
}

void LOGE(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    s_log.write(LogLevel::ERROR, format, args);
    va_end(args);
}

void ASSERT_MAIN_THREAD(const char* message)
{
    if( !core::task::is_main_thread() )
        LOGE("%s only run on the main thread.", message);
}

NS_LEMON_END