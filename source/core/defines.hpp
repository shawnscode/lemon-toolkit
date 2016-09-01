// @date 2016/07/28
// @author Mao Jingkai(oammix@gmail.com)

#pragma once

#include <defines.hpp>

#define NS_LEMON_CORE_BEGIN namespace lemon { namespace core {
#define NS_LEMON_CORE_END } }

NS_LEMON_CORE_BEGIN

static const size_t kEntPoolChunkSize = 512;
static const size_t kEntMaxComponents = 128;
static const size_t kDbgMaxTracebackFrames = 63;

struct EntityManager;

NS_LEMON_CORE_END