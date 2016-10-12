// @date 2016/09/29
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <core/event.hpp>
#include <core/ecs.hpp>
#include <core/subsystem.hpp>
#include <core/task.hpp>

NS_LEMON_CORE_BEGIN

bool initialize(unsigned nworker);
bool is_running();
void dispose();

NS_LEMON_CORE_END