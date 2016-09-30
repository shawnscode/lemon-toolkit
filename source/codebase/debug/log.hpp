// @date 2016/06/07
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>
#include <iosfwd>

NS_LEMON_BEGIN

enum class LogLevel : uint8_t
{
    VERBOSE,
    INFORMATION,
    WARNING,
    ERROR,
};

void set_output_stream(LogLevel, std::ostream*);

NS_LEMON_END