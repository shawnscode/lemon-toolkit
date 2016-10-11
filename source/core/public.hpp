// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/event.hpp>
#include <core/entity.hpp>
#include <core/subsystem.hpp>
#include <core/task.hpp>

NS_LEMON_CORE_BEGIN

bool initialize(unsigned nworker);
void dispose();

NS_LEMON_CORE_END