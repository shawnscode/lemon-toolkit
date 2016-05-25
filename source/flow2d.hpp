#pragma once

#define NS_FLOW2D_BEGIN namespace flow2d {
#define NS_FLOW2D_END }

#define USING_NS_FLOW2D using namespace flow2d;

// PRECOMPILED HEADERS
#include <cstdint>
#include <cassert>
#include <functional>
#include <memory>

// FORWARD DECLARATIONS
NS_FLOW2D_BEGIN

#define INLINE inline

// ENTITY COMPONENTS SYSTEM CONFIGS
static const int32_t kEntMaxComponents = 64;

struct Component;
template<typename T> struct ComponentTrait;
template<typename T> struct ComponentHandle;

struct Entity;
struct EntityIterator;
struct EntityView;
template<typename ...T> struct EntityViewTrait;
struct EntityManager;

NS_FLOW2D_END