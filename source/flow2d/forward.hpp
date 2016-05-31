#pragma once

#define NS_FLOW2D_BEGIN namespace flow2d {
#define NS_FLOW2D_END }

// PRECOMPILED HEADERS
#include <cstdint>
#include <cassert>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <bitset>
#include <vector>
#include <unordered_map>

NS_FLOW2D_BEGIN

#define INLINE inline

/// GLOBAL CONFIGS
static const int32_t kEntMaxComponents = 64;

/// FORWARD DECLARATIONS

//- ENTITY COMPONENT SYSTEM
struct Component;
template<typename T> struct ComponentTrait;
template<typename T> struct ComponentHandle;

struct Entity;
struct EntityIterator;
struct EntityView;
template<typename ...T> struct EntityViewTrait;
struct EntityManager;

struct System;
template<typename T> struct SystemTrait;
struct SystemManager;

//- EVENT DISPATCH SYSTEM
struct Event;
template<typename T> struct EventTrait;
struct EventManager;

//- SCENE GRAPH SYSTEM
struct Transform;
struct SceneSystem;
struct ActionSystem;

NS_FLOW2D_END