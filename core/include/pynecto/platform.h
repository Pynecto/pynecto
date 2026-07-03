#ifndef PYNECTO_PLATFORM_H
#define PYNECTO_PLATFORM_H

#include <stddef.h>
#include <stdint.h>   
#include <stdbool.h>
#include <assert.h>
#include <inttypes.h>

#if defined(_MSC_VER)
#   define PNC_COMPILER_MSVC    1
#   define PNC_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
#   define PNC_COMPILER_CLANG   1
#   define PNC_COMPILER_VERSION (__clang_major__ * 10000 \
                                + __clang_minor__ * 100  \
                                + __clang_patchlevel__)
#elif defined(__GNUC__)
#   define PNC_COMPILER_GCC     1
#   define PNC_COMPILER_VERSION (__GNUC__ * 10000 \
                                + __GNUC_MINOR__ * 100 \
                                + __GNUC_PATCHLEVEL__)
#else
#   error "[Pynecto] Unsupported compiler. Use GCC, Clang, or MSVC."
#endif


#if defined(PNC_PLATFORM_WINDOWS)
#   define PNC_PLATFORM_NAME    "windows"
#   define PNC_PLATFORM_DESKTOP 1
#   ifndef NOMINMAX
#       define NOMINMAX
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN
#   endif

#elif defined(PNC_PLATFORM_MACOS)
#   define PNC_PLATFORM_NAME    "macos"
#   define PNC_PLATFORM_DESKTOP 1

#elif defined(PNC_PLATFORM_LINUX)
#   define PNC_PLATFORM_NAME    "linux"
#   define PNC_PLATFORM_DESKTOP 1

#else
#   error "[Pynecto] Unknown platform. Set PNC_PLATFORM_WINDOWS, \
PNC_PLATFORM_MACOS, or PNC_PLATFORM_LINUX via CMake."
#endif

#if defined(__x86_64__) || defined(_M_X64)
#   define PNC_ARCH_X64   1
#   define PNC_ARCH_NAME  "x86_64"
#   define PNC_64BIT      1
#elif defined(__i386__) || defined(_M_IX86)
#   define PNC_ARCH_X86   1
#   define PNC_ARCH_NAME  "x86"
#   define PNC_32BIT      1
#elif defined(__aarch64__) || defined(_M_ARM64)
#   define PNC_ARCH_ARM64 1
#   define PNC_ARCH_NAME  "arm64"
#   define PNC_64BIT      1
#elif defined(__arm__) || defined(_M_ARM)
#   define PNC_ARCH_ARM32 1
#   define PNC_ARCH_NAME  "arm32"
#   define PNC_32BIT      1
#else
#   define PNC_ARCH_NAME  "unknown"
#   warning "[Pynecto] Unrecognised CPU architecture — some features may not work."
#endif


#if defined(PNC_COMPILER_MSVC)
#   if defined(PNC_CORE_BUILD)
#       define PNC_API __declspec(dllexport)
#   else
#       define PNC_API __declspec(dllimport)
#   endif
#else
#   if defined(PNC_CORE_BUILD)
#       define PNC_API __attribute__((visibility("default")))
#   else
#       define PNC_API
#   endif
#endif

#if defined(PNC_COMPILER_MSVC)
#   define PNC_INLINE __forceinline
#else
#   define PNC_INLINE __attribute__((always_inline)) inline
#endif

#if defined(PNC_COMPILER_MSVC)
#   define PNC_NOINLINE __declspec(noinline)
#else
#   define PNC_NOINLINE __attribute__((noinline))
#endif

#if defined(PNC_COMPILER_GCC) || defined(PNC_COMPILER_CLANG)
#   define PNC_LIKELY(x)   __builtin_expect(!!(x), 1)
#   define PNC_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define PNC_LIKELY(x)   (x)
#   define PNC_UNLIKELY(x) (x)
#endif

#if defined(PNC_COMPILER_MSVC)
#   define PNC_UNREACHABLE() __assume(0)
#elif defined(PNC_COMPILER_GCC) || defined(PNC_COMPILER_CLANG)
#   define PNC_UNREACHABLE() __builtin_unreachable()
#else
#   define PNC_UNREACHABLE() ((void)0)
#endif

#if defined(PNC_COMPILER_MSVC)
#   define PNC_DEPRECATED(msg) __declspec(deprecated(msg))
#else
#   define PNC_DEPRECATED(msg) __attribute__((deprecated(msg)))
#endif

typedef uint8_t     pnc_u8;
typedef uint16_t    pnc_u16;
typedef uint32_t    pnc_u32;
typedef uint64_t    pnc_u64;

typedef int8_t      pnc_i8;
typedef int16_t     pnc_i16;
typedef int32_t     pnc_i32;
typedef int64_t     pnc_i64;

typedef float       pnc_f32;
typedef double      pnc_f64;

typedef bool        pnc_bool;
typedef uint8_t     pnc_byte;
typedef size_t      pnc_usize;
typedef ptrdiff_t   pnc_isize;

static_assert(sizeof(pnc_u8)  == 1, "pnc_u8 must be 1 byte");
static_assert(sizeof(pnc_u16) == 2, "pnc_u16 must be 2 bytes");
static_assert(sizeof(pnc_u32) == 4, "pnc_u32 must be 4 bytes");
static_assert(sizeof(pnc_u64) == 8, "pnc_u64 must be 8 bytes");
static_assert(sizeof(pnc_f32) == 4, "pnc_f32 must be 4 bytes");
static_assert(sizeof(pnc_f64) == 8, "pnc_f64 must be 8 bytes");

#define PNC_TRUE  true
#define PNC_FALSE false
#define PNC_NULL  NULL

#define PNC_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define PNC_UNUSED(x) ((void)(x))

#define PNC_CLAMP(val, lo, hi) \
    ((val) < (lo) ? (lo) : ((val) > (hi) ? (hi) : (val)))

/* Plain ternaries — GNU statement-expressions were rejected because
 * C_EXTENSIONS is OFF and -Wpedantic is on. Caveat: a/b are evaluated
 * twice, so callers must not pass expressions with side effects. */
#define PNC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PNC_MAX(a, b) ((a) > (b) ? (a) : (b))

#define PNC_ALIGN_UP(size, align) \
    (((size) + (align) - 1) & ~((align) - 1))

#define PNC_IS_POWER_OF_TWO(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

#define PNC_OFFSETOF(type, field) offsetof(type, field)

#define PNC_KB(n) ((pnc_usize)(n) * 1024ULL)
#define PNC_MB(n) ((pnc_usize)(n) * 1024ULL * 1024ULL)
#define PNC_GB(n) ((pnc_usize)(n) * 1024ULL * 1024ULL * 1024ULL)

#if defined(PNC_DEBUG)
#   include <stdio.h>
#   define PNC_ASSERT(cond)                                                 \
        do {                                                                \
            if (PNC_UNLIKELY(!(cond))) {                                   \
                fprintf(stderr,                                             \
                    "[Pynecto] ASSERT FAILED: %s\n"                        \
                    "  File : %s\n"                                         \
                    "  Line : %d\n"                                         \
                    "  Func : %s\n",                                        \
                    #cond, __FILE__, __LINE__, __func__);                   \
                assert(false);                                              \
            }                                                               \
        } while (0)
#else
#   define PNC_ASSERT(cond) ((void)0)
#endif


#define PNC_STATIC_ASSERT(cond, msg) static_assert(cond, msg)

#define PNC_VERSION_MAJOR 0
#define PNC_VERSION_MINOR 1
#define PNC_VERSION_PATCH 0

#define PNC_VERSION \
    ((PNC_VERSION_MAJOR * 10000) + (PNC_VERSION_MINOR * 100) + PNC_VERSION_PATCH)

#define PNC_VERSION_STRING "0.1.0"

#endif