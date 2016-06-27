#include <core/debug.hpp>
#include <cassert>

NS_FLOW2D_BEGIN

static void abortx(const char* file, int line, const char* format, va_list args)
{
    FLOW_LOGEV(format, args);
    FLOW_LOGE("\n\tIn: %s:%d\n\nStacktrace:", file, line);
    Debug::traceback();
    exit(0);
}

void ABORT(const char* file, int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    abortx(file, line, format, args);
    va_end(args);
}

void LOGI(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    Debug::log(LogLevel::INFORMATION, format, args);
    va_end(args);
}

void LOGW(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    Debug::log(LogLevel::WARNING, format, args);
    va_end(args);
}

void LOGE(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    Debug::log(LogLevel::ERROR, format, args);
    va_end(args);
}

void Debug::log(LogLevel level, const char* msg, va_list args)
{
    char buf[8192];
    int len = vsnprintf(buf, sizeof(buf), msg, args);
    buf[len] = '\0';

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

    fputs(stt[(int)level], stdout);
    fputs(buf, stdout);
    fputs(kANSIReset, stdout);
    fputs("\n", stdout);
    fflush(stdout);
}

void Debug::log(LogLevel level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log(level, format, args);
    va_end(args);
}

void Debug::abort(const char* file, int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    abortx(file, line, format, args);
    va_end(args);
}

// implementations for posix based operation systems

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <cxxabi.h>

static const char* addr2line(const char* addr, char* line, int len)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "addr2line -e /proc/%u/exe %s", getpid(), addr);
    FILE* f = popen(buf, "r");
    if (f)
    {
        fgets(line, len, f);
        line[strlen(line) - 1] = '\0';
        pclose(f);
        return line;
    }
    return "<addr2line missing>";
}

void Debug::traceback()
{
    // storage array for stack trace address data
    void* addrlist[kDbgMaxTracebackFrames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
    if (addrlen == 0)
    {
        FLOW_LOGE("\t<empty, possibly corrupt>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++)
    {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p)
        {
            if( *p == '(' ) begin_name = p;
            else if( *p == '+' ) begin_offset = p;
            else if( *p == ')' && begin_offset )
            {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset
            && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name,
                            funcname, &funcnamesize, &status);
            if( status == 0 )
            {
                funcname = ret; // use possibly realloc()-ed string
                FLOW_LOGE("\t%s : %s+%s", symbollist[i], funcname, begin_offset);
            }
            else
            {
                // demangling failed. Output function name as a C function with
                // no arguments.
                FLOW_LOGE("\t%s : %s()+%s", symbollist[i], begin_name, begin_offset);
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            FLOW_LOGE("\t%s", symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
}

NS_FLOW2D_END