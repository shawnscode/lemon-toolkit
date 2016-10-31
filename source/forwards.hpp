#pragma once

#include <cstdint>
#include <cstddef>

#if defined(IOS)
#define PLATFORM_IOS
#elif defined(ANDROID)
#define PLATFORM_ANDROID
#elif defined(_WIN32)
#define PLATFORM_WIN32
#else
#define PLATFORM_UNIX
#endif

#define INLINE inline

// STANDARD ENGINE COLLECTIONS
#define NS_LEMON_BEGIN namespace lemon {
#define NS_LEMON_END }

struct SDL_Window;

NS_LEMON_BEGIN

static const unsigned kDbgMaxTracebackFrames = 63;

extern void ABORT(const char* file, int line, const char* format, ...);
extern void LOGI(const char* format, ...);
extern void LOGW(const char* format, ...);
extern void LOGE(const char* format, ...);

#define ASSERT(condition, format, ...) do { \
    if( !(condition) ) { \
        ABORT(__FILE__, __LINE__, "\nAssertion failed: %s, " format "", #condition, ##__VA_ARGS__);\
    } \
} while(0)

#define FATAL(format, ...) ASSERT(false, format, ##__VA_ARGS__)
#define ENSURE(condition) ASSERT(condition, "")
#define NOTUSED(v) for(;;) { (void)(1 ? (void)0 : ((void)v)); break; }

static const unsigned kMaxRenderLayer = 32;

NS_LEMON_END

// MATH COLLECTIONS
#define NS_LEMON_MATH_BEGIN namespace lemon { namespace math {
#define NS_LEMON_MATH_END } }

// CORE COLLECTIONS
#define NS_LEMON_CORE_BEGIN namespace lemon { namespace core {
#define NS_LEMON_CORE_END } }

NS_LEMON_CORE_BEGIN

static const unsigned kEntPoolChunkSize = 128;
static const unsigned kEntMaxComponents = 64;
struct EntityComponentSystem;

NS_LEMON_CORE_END

// GRAPHICS COLLECTIONS
#define NS_LEMON_GRAPHICS_BEGIN namespace lemon { namespace graphics {
#define NS_LEMON_GRAPHICS_END } }

NS_LEMON_GRAPHICS_BEGIN

const static unsigned kMaxRenderDrawCall = 1024;

const static unsigned kMaxProgram = 32;

const static unsigned kMaxTexturePerMaterial = 8;
const static unsigned kMaxUniformsPerMaterial = 32;

const static unsigned kMaxUniforms = 1024;
const static unsigned kMaxVertexBuffer = 256;
const static unsigned kMaxIndexBuffer = 256;
const static unsigned kMaxTexture = 64;
const static unsigned kMaxRenderState = 32;

struct RenderFrame;
struct RenderFrontend;
struct WindowDevice;

NS_LEMON_GRAPHICS_END

// FILESYSTEM COLLECTIONS
#define NS_LEMON_FILESYSTEM_BEGIN namespace lemon { namespace fs {
#define NS_LEMON_FILESYSTEM_END }}

NS_LEMON_FILESYSTEM_BEGIN

const static unsigned kMaxPath = 256;
struct Path;
struct File;
struct Filesystem;

NS_LEMON_FILESYSTEM_END

// RESOURCE COLLECTIONS
#define NS_LEMON_RESOURCE_BEGIN namespace lemon { namespace res {
#define NS_LEMON_RESOURCE_END } }

NS_LEMON_RESOURCE_BEGIN

const static unsigned kCacheDefaultThreshold = 128 * 1024 * 1024;
struct Archive;
struct FilesystemArchive;
struct PackageArchive;
struct ArchiveCollection;

NS_LEMON_RESOURCE_END

