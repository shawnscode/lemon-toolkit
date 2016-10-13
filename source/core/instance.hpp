// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <forwards.hpp>

NS_LEMON_CORE_BEGIN

enum class Status : uint8_t
{
    IDLE,
    RUNNING,
    DISPOSED
};

bool initialize(unsigned nworker);
Status status();
void dispose();

NS_LEMON_CORE_END