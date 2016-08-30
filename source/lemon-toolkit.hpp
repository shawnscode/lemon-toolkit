#pragma once

#include <core/context.hpp>
#include <core/entity.hpp>
#include <core/event.hpp>
#include <core/memory.hpp>
#include <core/typeinfo.hpp>
#include <core/scheduler.hpp>

#include <math/vector.hpp>
#include <math/matrix.hpp>
#include <math/quaternion.hpp>
#include <math/rect.hpp>
#include <math/color.hpp>
#include <math/string_hash.hpp>

#include <graphics/backend.hpp>
#include <graphics/shader.hpp>
#include <graphics/index_buffer.hpp>
#include <graphics/vertex_buffer.hpp>
#include <graphics/texture.hpp>

#include <resource/path.hpp>
#include <resource/filesystem.hpp>
#include <resource/archives.hpp>
#include <resource/resource.hpp>
#include <resource/image.hpp>

#include <scene/transform.hpp>

#include <engine/input.hpp>
#include <engine/engine.hpp>
#include <engine/application.hpp>

#define USING_NS_LEMON using namespace lemon;
#define USING_NS_LEMON_CORE using namespace lemon::core;
#define USING_NS_LEMON_MATH using namespace lemon::math;
#define USING_NS_LEMON_GRAPHICS using namespace lemon::graphics;
#define USING_NS_LEMON_RESOURCE using namespace lemon::res;
#define USING_NS_LEMON_FILESYSTEM using namespace lemon::fs;