#ifndef DEFINES_H
#define DEFINES_H

#define APPLICATION_NAME "tire auralization"

#if defined(__clang__)
#	define static_assert _Static_assert
#else
#	define static_assert static_assert
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#	define PLATFORM_WINDOWS 1
#	ifndef _WIN64
#		error "64 bit is required"
#	endif
#elif defined(__linux__) || defined(__gnu_linus__)
#	define PLATFORM_LINUX 1
#	if defined(__ANDROID)
#		define PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
#	define PLATFORM_UNIX
#elif defined(_POSIX_VERSION)
#	define PLATFORM_POSIX
#elif
#	error "unknown platform"
#endif

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

typedef float f32;
typedef double f64;

typedef unsigned char b8;
typedef signed int b32;

static_assert(sizeof(u8 ) == 1, "expected u8 to be 1 byte");
static_assert(sizeof(u16) == 2, "expected u16 to be 2 bytes");
static_assert(sizeof(u32) == 4, "expected u32 to be 4 bytes");
static_assert(sizeof(u64) == 8, "expected u64 to be 8 bytes");

static_assert(sizeof(i8 ) == 1, "expected i8 to be 1 byte");
static_assert(sizeof(i16) == 2, "expected i16 to be 2 bytes");
static_assert(sizeof(i32) == 4, "expected i32 to be 4 bytes");
static_assert(sizeof(i64) == 8, "expected i64 to be 8 bytes");

static_assert(sizeof(f32) == 4, "expected f32 to be 4 bytes");
static_assert(sizeof(f64) == 8, "expected f64 to be 8 bytes");

static_assert(sizeof(b8) == 1, "expected b8 to be 1 byte");
static_assert(sizeof(b32) == 4, "expected b32 to be 4 bytes");

#define U8_MAX  (255U)
#define U16_MAX (65535U)
#define U32_MAX (4294967295U)
#define U64_MAX (18446744073709551615ULL)

#define I8_MAX  (127)
#define I16_MAX (32767)
#define I32_MAX (2147483647)
#define I64_MAX (9223372036854775807LL)

#ifndef NULL
#	define NULL 0
#endif 

#if defined(__has_builtin)
#	if __has_builtin(__builtin_unreachable)
#		define unreachable() log::_message_log_with_location(log::LEVEL_AMBIGUOUS, __FILE__, __LINE__, "unreachable: how did we get here?"); __builtin_unreachable()
#	endif
#elif defined(_MSC_VER)
#	define unreachable() log::_message_log_with_location(log::LEVEL_AMBIGUOUS, __FILE__, __LINE__, "unreachable: how did we get here?"); __assume(0)
#else
#	define unreachable() log::_message_log_with_location(log::LEVEL_AMBIGUOUS, __FILE__, __LINE__, "unreachable: how did we get here?")
#endif

#define assert(condition, fmt, ...) if (!(condition)) {log::_assert_failure_log_with_location(__FILE__, __LINE__, #condition, fmt, ##__VA_ARGS__); *(volatile int*)0 = 0;}
#define unimplemented() log::_message_log_with_location(log::LEVEL_AMBIGUOUS, __FILE__, __LINE__, "unimplemented")

#define OUT

// NOTE(zgayford): for functions, signifies I'm meaning to use the 
// static keyword for internal linkage
#define internal static

typedef struct Slice {
	void* ptr;
	u64 length;
} Slice;

typedef union v2i {
	struct { i32 x, y; };
	struct { i32 u, v; };
	struct { i32 width, height; };
	i32 e[2];
} v2i;

#define CLAMP(value, max, min) (((value) <= (min)) ? (min) : ((value) >= (max)) ? (max) : (value))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define GIBIBYTES(amt) ((amt) * 1024ULL * 1024ULL * 1024ULL)
#define MEBIBYTES(amt) ((amt) * 1024ULL * 1024ULL)
#define KIBIBYTES(amt) ((amt) * 1024ULL)

#define GIGABYTES(amt) ((amt) * 1000ULL * 1000ULL * 1000ULL)
#define MEGABYTES(amt) ((amt) * 1000ULL * 1000ULL)
#define KILOBYTES(amt) ((amt) * 1000ULL)

#endif // DEFINES_H
