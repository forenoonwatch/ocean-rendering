#pragma once

#include <cstdint>
#include <cstdio>
#include <stdexcept>

#if defined(__clang__)
	#define COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
	#define COMPILER_GCC
#elif defined(_MSC_VER)
	#define COMPILER_MSVC
#else
	#define COMPILER_OTHER
#endif

typedef uint8_t uint8;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef intptr_t intptr;
typedef uintptr_t uintptr;

#ifdef COMPILER_MSVC
	#define FORCEINLINE __forceinline
	#define PACKED
#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
	#define FORCEINLINE inline __attribute__ ((always_inline))
	#define PACKED __attribute__((packed, aligned(4)))
#else
	#define FORCEINLINE inline
	#define PACKED
#endif

#define NULL_COPY_AND_ASSIGN(T) \
	T(T&& other) = delete; \
	T(const T& other) = delete; \
	void operator=(const T& other) = delete;

#define LOG_ERROR "Error"
#define LOG_WARNING "Warning"

#define DEBUG_LOG(category, level, message, ...) \
	fprintf(stderr, "[%s] ", category); \
	fprintf(stderr, "[%s] (%s:%d): ", level, __FILE__, __LINE__); \
	fprintf(stderr, message, ##__VA_ARGS__); \
	fprintf(stderr, "\n")
#define DEBUG_LOG_TEMP(message, ...) DEBUG_LOG("TEMP", "TEMP", message, ##__VA_ARGS__)
#define DEBUG_LOG_TEMP2(message) DEBUG_LOG("TEMP", "TEMP", "%s", message)

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

