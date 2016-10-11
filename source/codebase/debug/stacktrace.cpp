// @date 2016/09/05
// @author Mao Jingkai(oammix@gmail.com)

#include <codebase/debug/stacktrace.hpp>

#include <iostream>

NS_LEMON_BEGIN

// implementations for posix based operation systems
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <cxxabi.h>

::std::ostream& stacktrace(::std::ostream& out, unsigned skip)
{
    // storage array for stack trace address data
    void* addrlist[kDbgMaxTracebackFrames+1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
    if (addrlen == 0)
        return out << "    <empty, possibly corrupt>\n";

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    char buffer[4096];
    for (int i = skip; i < addrlen; i++)
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
            funcname = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);

            sprintf(buffer, "    - %s: (%s)+%s %s\n",
                symbollist[i],
                status == 0 ? funcname : begin_name,
                begin_offset,
                end_offset);

            out << buffer;
        }
        else
        {
            // couldn't parse the line? print the whole line.
            sprintf(buffer, "    - %s\n", symbollist[i]);
            out << buffer;
        }
    }

    free(funcname);
    free(symbollist);
    return out;
}

NS_LEMON_END