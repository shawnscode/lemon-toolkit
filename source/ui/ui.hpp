#pragma once

#include <forward.hpp>

#define NS_FLOW2D_UI_BEGIN namespace flow2d { namespace ui {
#define NS_FLOW2D_UI_END } }

NS_FLOW2D_UI_BEGIN

const static size_t kUiComponentsChunkSize = 128;
const static size_t kMaxEventListeners = 8;

struct Widget;
struct View;
struct Container;

struct CanvasDirector;
struct CavnasSystem;

NS_FLOW2D_UI_END