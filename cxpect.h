/*
 * cxpect.h - single header BDD testing library
 * define CXPECT_IMPLEMENTATION in exactly one translation unit before including this file
 */
#ifndef CXPECT_H
#define CXPECT_H

#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define cxpect_stringify_impl(x) #x
#define cxpect_stringify(x)		 cxpect_stringify_impl(x)
#define cxpect_macro_expand(x)	 x

#define cxpect_macro_concat_impl(a, b)			  a##b
#define cxpect_macro_concat(a, b)				  cxpect_macro_concat_impl(a, b)
#define cxpect_macro_concat_3_impl(a, b, c)		  a##b##c
#define cxpect_macro_concat_3(a, b, c)			  cxpect_macro_concat_3_impl(a, b, c)
#define cxpect_macro_concat_4_impl(a, b, c, d)	  a##b##c##d
#define cxpect_macro_concat_4(a, b, c, d)		  cxpect_macro_concat_4_impl(a, b, c, d)
#define cxpect_macro_concat_5_impl(a, b, c, d, e) a##b##c##d##e
#define cxpect_macro_concat_5(a, b, c, d, e)	  cxpect_macro_concat_5_impl(a, b, c, d, e)

#define cxpect_macro_variable(x) cxpect_macro_concat(x, __LINE__)

// clang-format off
#define cxpect_macro_reversed_n() 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define cxpect_macro_arg_n(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, n, ...) n

#define cxpect_macro_count_arg_impl(...) cxpect_macro_expand(cxpect_macro_arg_n(__VA_ARGS__))
#define cxpect_macro_count_arg(...) cxpect_macro_count_arg_impl(__VA_ARGS__, cxpect_macro_reversed_n())

#define cxpect_macro_has_comma(...) cxpect_macro_expand(cxpect_macro_arg_n(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0))
#define cxpect_macro_trigger_parenthesis(...) ,
#define cxpect_macro_is_empty_value_0001 ,
// clang-format on

#ifndef cxpect_typeof
	#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
		#define cxpect_typeof typeof
	#elif defined(__GNUC__) || defined(__clang__)
		#define cxpect_typeof __typeof__
	#else
		#error "cxpect requires C23 typeof or a compiler with __typeof__ support"
	#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define cxpect_unlikely(x) __builtin_expect(!!(x), 0)
#else
	#define cxpect_unlikely(x) (x)
#endif

typedef enum CxpectTerminalColor
{
	CxpectTerminalColorsCyan = 0,
	CxpectTerminalColorsBlue,
	CxpectTerminalColorsPurple,
	CxpectTerminalColorsGreen,
	CxpectTerminalColorsYellow,
	CxpectTerminalColorsBlack,
	CxpectTerminalColorsClear,
	CxpectTerminalColorsBrown,
	CxpectTerminalColorsRed,
	CxpectTerminalColorsNone,
	CxpectTerminalColorsCount,
} CxpectTerminalColor;

#ifndef CXPECT_NO_COLORED_OUTPUT
static const char* system_to_colors[] = {
	[CxpectTerminalColorsCyan] = "\x1b[1;36m",
	[CxpectTerminalColorsBlue] = "\x1b[1;34m",
	[CxpectTerminalColorsPurple] = "\x1b[1;35m",
	[CxpectTerminalColorsGreen] = "\x1b[1;32m",
	[CxpectTerminalColorsYellow] = "\x1b[1;33m",
	[CxpectTerminalColorsBlack] = "\x1b[1;30m",
	[CxpectTerminalColorsClear] = "\x1b[1;0m",
	[CxpectTerminalColorsBrown] = "\x1b[1;33m",
	[CxpectTerminalColorsRed] = "\x1b[1;31m",
	[CxpectTerminalColorsNone] = "",
};
#endif

static const char*
cxpect_get_terminal_color(CxpectTerminalColor color)
{
#ifndef CXPECT_NO_COLORED_OUTPUT
	const uint32_t color_idx = (uint32_t) color >= CxpectTerminalColorsCount ? CxpectTerminalColorsNone : color;
	return system_to_colors[color_idx];
#else
	(void) color;
	return "";
#endif
}

#ifndef cxpect_fail_impl
	#if defined(__has_builtin)
		#if __has_builtin(__builtin_trap)
			#define cxpect_fail_impl __builtin_trap()
		#endif
	#endif

	#ifndef cxpect_fail_impl
		#if defined(_MSC_VER)
			#define cxpect_fail_impl __debugbreak()
		#elif defined(__GNUC__) || defined(__clang__)
			#define cxpect_fail_impl __builtin_trap()
		#else
			#define cxpect_fail_impl abort()
		#endif
	#endif
#endif

#ifndef cxpect_default_ctx
	#define cxpect_default_ctx cxpect_default_ctx
#endif

#define cxpect_fail                                                                                                    \
	do                                                                                                                 \
	{                                                                                                                  \
		if (cxpect_default_ctx->fault_env)                                                                             \
		{                                                                                                              \
			longjmp(*cxpect_default_ctx->fault_env, 1);                                                                \
		}                                                                                                              \
		else                                                                                                           \
		{                                                                                                              \
			cxpect_fail_impl;                                                                                          \
		}                                                                                                              \
	} while (0)

// formatting

static inline void
cxpect_print_char(FILE* file, char value)
{
	fprintf(file, "'%c'", value);
}

static inline void
cxpect_print_signed_char(FILE* file, signed char value)
{
	fprintf(file, "%d", (int) value);
}

static inline void
cxpect_print_unsigned_char(FILE* file, unsigned char value)
{
	fprintf(file, "%u", (unsigned int) value);
}

static inline void
cxpect_print_short(FILE* file, short value)
{
	fprintf(file, "%d", (int) value);
}

static inline void
cxpect_print_unsigned_short(FILE* file, unsigned short value)
{
	fprintf(file, "%u", (unsigned int) value);
}

static inline void
cxpect_print_int(FILE* file, int value)
{
	fprintf(file, "%d", value);
}

static inline void
cxpect_print_unsigned_int(FILE* file, unsigned int value)
{
	fprintf(file, "%u", value);
}

static inline void
cxpect_print_long(FILE* file, long value)
{
	fprintf(file, "%ld", value);
}

static inline void
cxpect_print_unsigned_long(FILE* file, unsigned long value)
{
	fprintf(file, "%lu", value);
}

static inline void
cxpect_print_long_long(FILE* file, long long value)
{
	fprintf(file, "%lld", value);
}

static inline void
cxpect_print_unsigned_long_long(FILE* file, unsigned long long value)
{
	fprintf(file, "%llu", value);
}

static inline void
cxpect_print_float(FILE* file, float value)
{
	fprintf(file, "%.9g", (double) value);
}

static inline void
cxpect_print_double(FILE* file, double value)
{
	fprintf(file, "%.17g", value);
}

static inline void
cxpect_print_long_double(FILE* file, long double value)
{
	fprintf(file, "%.21Lg", value);
}

static inline void
cxpect_print_bool(FILE* file, bool value)
{
	fputs(value ? "true" : "false", file);
}

static inline void
cxpect_print_cstr(FILE* file, const char* value)
{
	if (value == NULL)
	{
		fputs("NULL", file);
		return;
	}
	fprintf(file, "\"%s\"", value);
}

static inline void
cxpect_print_ptr(FILE* file, const void* value)
{
	if (value == NULL)
	{
		fputs("NULL", file);
		return;
	}
#ifdef UINTPTR_MAX
	fprintf(file, "0x%" PRIxPTR, (uintptr_t) value);
#else
	fputs("<pointer>", file);
#endif
}

static inline void
cxpect_print_complex(FILE* file, ...)
{
	fputs("<complex type>", file);
}

#ifndef CXPECT_CUSTOM_FORMATTERS
	#define CXPECT_CUSTOM_FORMATTERS
#endif

#define cxpect_print_generic_val(file_, value_)                                                                        \
	_Generic((value_),                                                                                                 \
		char: cxpect_print_char,                                                                                       \
		signed char: cxpect_print_signed_char,                                                                         \
		unsigned char: cxpect_print_unsigned_char,                                                                     \
		short: cxpect_print_short,                                                                                     \
		unsigned short: cxpect_print_unsigned_short,                                                                   \
		int: cxpect_print_int,                                                                                         \
		unsigned int: cxpect_print_unsigned_int,                                                                       \
		long: cxpect_print_long,                                                                                       \
		unsigned long: cxpect_print_unsigned_long,                                                                     \
		long long: cxpect_print_long_long,                                                                             \
		unsigned long long: cxpect_print_unsigned_long_long,                                                           \
		float: cxpect_print_float,                                                                                     \
		double: cxpect_print_double,                                                                                   \
		long double: cxpect_print_long_double,                                                                         \
		char*: cxpect_print_cstr,                                                                                      \
		const char*: cxpect_print_cstr,                                                                                \
		void*: cxpect_print_ptr,                                                                                       \
		const void*: cxpect_print_ptr,                                                                                 \
		bool: cxpect_print_bool,                                                                                       \
		CXPECT_CUSTOM_FORMATTERS default: cxpect_print_complex)((file_), (value_))

// context

#ifdef CXPECT_ENABLE_RANDOM
typedef struct cxpect_pcg_state
{
	uint64_t state;
	uint64_t inc;
} cxpect_pcg_state_t;
#endif

struct cxpect_ctx;

typedef void (*cxpect_test_fn)(struct cxpect_ctx*);
typedef void (*cxpect_before_fn)(struct cxpect_ctx*);
typedef void (*cxpect_after_fn)(struct cxpect_ctx*);

#ifndef CXPECT_CUSTOM_CONTEXT_MEMBERS
	#define CXPECT_CUSTOM_CONTEXT_MEMBERS
#endif

typedef struct cxpect_ctx
{
	char** argv;
	char* filter;

#ifdef CXPECT_ENABLE_RANDOM
	cxpect_pcg_state_t rng_state;
	uint64_t seed[2];
#endif

	cxpect_before_fn before_each_cb;
	cxpect_after_fn after_each_cb;
	jmp_buf* fault_env;

	uint32_t tests_passed;
	uint32_t tests_failed;
	int64_t argc;

	CXPECT_CUSTOM_CONTEXT_MEMBERS
} cxpect_ctx_t;

typedef struct
{
	const char* name;
	cxpect_test_fn fn;
} cxpect_case_t;

void
cxpect_run_tests(cxpect_ctx_t* ctx, const cxpect_case_t* cases);

void
cxpect_ctx_post_init(cxpect_ctx_t* ctx);

#define cxpect_before_each(fn)                                                                                         \
	do                                                                                                                 \
	{                                                                                                                  \
		cxpect_default_ctx->before_each_cb = fn;                                                                       \
	} while (0)

#define cxpect_after_each(fn)                                                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		cxpect_default_ctx->after_each_cb = fn;                                                                        \
	} while (0)

#define cxpect_clear_hooks()                                                                                           \
	do                                                                                                                 \
	{                                                                                                                  \
		cxpect_default_ctx->before_each_cb = NULL;                                                                     \
		cxpect_default_ctx->after_each_cb = NULL;                                                                      \
	} while (0)

#define cxpect_describe(name, ...)                                                                                     \
	do                                                                                                                 \
	{                                                                                                                  \
		const char* const cxpect_macro_variable(cxpect_internal_describe_name) = (name);                               \
		fprintf(stdout,                                                                                                \
			"\n%s[DESCRIBE]%s %s\n",                                                                                   \
			cxpect_get_terminal_color(CxpectTerminalColorsBlue),                                                       \
			cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                      \
			cxpect_macro_variable(cxpect_internal_describe_name) != NULL                                               \
				? cxpect_macro_variable(cxpect_internal_describe_name)                                                 \
				: "(unnamed)");                                                                                        \
		__VA_ARGS__                                                                                                    \
	} while (0)

#define cxpect_it(name, ...)                                                                                           \
	do                                                                                                                 \
	{                                                                                                                  \
		volatile bool cxpect_internal_failed = false;                                                                  \
		jmp_buf cxpect_internal_test_env;                                                                              \
		jmp_buf cxpect_internal_after_env;                                                                             \
		jmp_buf* const cxpect_internal_previous_env = cxpect_default_ctx->fault_env;                                   \
		const char* const cxpect_internal_test_name = (name);                                                          \
		(void) fprintf(                                                                                                \
			stdout, "    [IT] %-45s ", cxpect_internal_test_name != NULL ? cxpect_internal_test_name : "(unnamed)");   \
		cxpect_default_ctx->fault_env = &cxpect_internal_test_env;                                                     \
		if (setjmp(cxpect_internal_test_env) == 0)                                                                     \
		{                                                                                                              \
			if (cxpect_default_ctx->before_each_cb != NULL)                                                            \
			{                                                                                                          \
				cxpect_default_ctx->before_each_cb(cxpect_default_ctx);                                                \
			}                                                                                                          \
			__VA_ARGS__                                                                                                \
		}                                                                                                              \
		else                                                                                                           \
		{                                                                                                              \
			cxpect_internal_failed = true;                                                                             \
		}                                                                                                              \
		if (cxpect_default_ctx->after_each_cb != NULL)                                                                 \
		{                                                                                                              \
			cxpect_default_ctx->fault_env = &cxpect_internal_after_env;                                                \
			if (setjmp(cxpect_internal_after_env) == 0)                                                                \
			{                                                                                                          \
				cxpect_default_ctx->after_each_cb(cxpect_default_ctx);                                                 \
			}                                                                                                          \
			else                                                                                                       \
			{                                                                                                          \
				cxpect_internal_failed = true;                                                                         \
			}                                                                                                          \
		}                                                                                                              \
		cxpect_default_ctx->fault_env = cxpect_internal_previous_env;                                                  \
		if (cxpect_internal_failed)                                                                                    \
		{                                                                                                              \
			(void) fprintf(stdout,                                                                                     \
				"%s✗ FAILED%s\n",                                                                                      \
				cxpect_get_terminal_color(CxpectTerminalColorsRed),                                                    \
				cxpect_get_terminal_color(CxpectTerminalColorsClear));                                                 \
			cxpect_default_ctx->tests_failed++;                                                                        \
		}                                                                                                              \
		else                                                                                                           \
		{                                                                                                              \
			(void) fprintf(stdout,                                                                                     \
				"%s✓ OK%s\n",                                                                                          \
				cxpect_get_terminal_color(CxpectTerminalColorsGreen),                                                  \
				cxpect_get_terminal_color(CxpectTerminalColorsClear));                                                 \
			cxpect_default_ctx->tests_passed++;                                                                        \
		}                                                                                                              \
	} while (false)


#define cxpect_case(callback) ((cxpect_case_t) {.name = #callback, .fn = (callback)})

#define CXPECT_CASES(...)                                                                                              \
	((cxpect_case_t[]) {                                                                                               \
		__VA_ARGS__,                                                                                                   \
		{NULL, NULL},                                                                                                  \
	})

#define CXPECT_TEST(name) static void name(cxpect_ctx_t* cxpect_default_ctx)

#define CXPECT_RUN(cxpect_run_ctx, ...)                                                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		const cxpect_case_t cxpect_cases[] = {__VA_ARGS__, {NULL, NULL}};                                              \
		cxpect_run_tests(cxpect_run_ctx, cxpect_cases);                                                                \
	} while (0)

#define CXPECT_MAIN(...)                                                                                               \
	int main(int argc, char** argv)                                                                                    \
	{                                                                                                                  \
		cxpect_ctx_t cxpect_default_ctx = {                                                                            \
			.argc = (int64_t) argc,                                                                                    \
			.argv = argv,                                                                                              \
			.filter = argc > 1 ? argv[1] : NULL,                                                                       \
		};                                                                                                             \
		cxpect_ctx_post_init(&cxpect_default_ctx);                                                                     \
		CXPECT_RUN(&cxpect_default_ctx, __VA_ARGS__);                                                                  \
		return cxpect_default_ctx.tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;                                     \
	}

// float comparison

typedef union
{
	float f;
	uint32_t u;
	int32_t i;
} cxpect_math_mask_32;

typedef union
{
	double d;
	uint64_t u;
	int64_t i;
} cxpect_math_mask_64;

static inline float
cxpect_math_float_abs(float v)
{
	return v < 0.0f ? -v : v;
}

static inline double
cxpect_math_double_abs(double v)
{
	return v < 0.0 ? -v : v;
}

static inline long double
cxpect_math_long_double_abs(long double value)
{
	return value < 0.0L ? -value : value;
}

#ifndef CXPECT_FLOAT_ABS_EPSILON
	#define CXPECT_FLOAT_ABS_EPSILON 0.00001f
#endif
#ifndef CXPECT_DOUBLE_ABS_EPSILON
	#define CXPECT_DOUBLE_ABS_EPSILON 0.00001
#endif
#ifndef CXPECT_LONG_DOUBLE_ABS_EPSILON
	#define CXPECT_LONG_DOUBLE_ABS_EPSILON 0.00001L
#endif

#if defined(__clang__)
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wfloat-equal"
#elif defined(__GNUC__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

#ifdef CXPECT_ENABLE_ULP_COMPARE
_Static_assert(sizeof(float) == sizeof(uint32_t), "cxpect ULP comparison requires 32-bit float");
_Static_assert(sizeof(double) == sizeof(uint64_t), "cxpect ULP comparison requires 64-bit double");
_Static_assert(
	FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128, "cxpect ULP comparison requires IEEE-754 float");
_Static_assert(DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024, "cxpect ULP comparison requires IEEE-754 double");

	#define cxpect_math_float_exponent_mask 0x7f800000u
	#define cxpect_math_float_mantissa_mask 0x007fffffu
	#define cxpect_math_float_sign_mask		0x80000000u

	#define cxpect_math_pos_nan	 ((cxpect_math_mask_32) {.u = 0x7fffffffu})
	#define cxpect_math_neg_nan	 ((cxpect_math_mask_32) {.u = 0xffffffffu})
	#define cxpect_math_pos_inf	 ((cxpect_math_mask_32) {.u = 0x7f800000u})
	#define cxpect_math_neg_inf	 ((cxpect_math_mask_32) {.u = 0xff800000u})
	#define cxpect_math_eps_bits ((cxpect_math_mask_32) {.u = 0x34000000u})
	#ifndef cxpect_math_comparison_eps5_bits
		#define cxpect_math_comparison_eps5_bits ((cxpect_math_mask_32) {.f = 0.00001f})
	#endif

	#ifndef CXPECT_MATCH_NAN_EQUALS_NAN
		#define CXPECT_MATCH_NAN_EQUALS_NAN 0
	#endif

static inline uint32_t
cxpect_math_float_almost_zero(cxpect_math_mask_32 self)
{
	return ((self.u & (cxpect_math_float_exponent_mask | cxpect_math_float_mantissa_mask)) == 0) ||
		   (cxpect_math_float_abs(self.f) <= cxpect_math_eps_bits.f);
}

static inline uint32_t
cxpect_math_float_ulp_distance(const cxpect_math_mask_32 ileft, const cxpect_math_mask_32 iright)
{
	return (ileft.u > iright.u ? ileft.u : iright.u) - (ileft.u > iright.u ? iright.u : ileft.u);
}

static inline uint32_t
cxpect_math_float_is_nan(cxpect_math_mask_32 self)
{
	return (self.u & 0x7fffffffu) > 0x7f800000u;
}

static inline uint32_t
cxpect_math_float_is_inf(cxpect_math_mask_32 self)
{
	return (self.u & 0x7fffffffu) == 0x7f800000u;
}

static inline bool
cxpect_math_float_exact_abs_match(cxpect_math_mask_32 left, cxpect_math_mask_32 right)
{
	const uint32_t abs_mask = cxpect_math_float_exponent_mask | cxpect_math_float_mantissa_mask;
	return (left.u & abs_mask) == (right.u & abs_mask);
}

/*
 * ULP comparison requires an IEEE 754 float representation
 */
static inline bool
cxpect_math_float_equals(const float left, const float right)
{
	const cxpect_math_mask_32 ileft = {.f = left};
	const cxpect_math_mask_32 iright = {.f = right};

	const uint32_t diff_sign = (ileft.u ^ iright.u) >> 31;
	const uint32_t both_zero = cxpect_math_float_almost_zero(ileft) & cxpect_math_float_almost_zero(iright);
	const uint32_t exact = (ileft.u == iright.u);
	const uint32_t dist = cxpect_math_float_ulp_distance(ileft, iright);

	const uint32_t l_nan = cxpect_math_float_is_nan(ileft);
	const uint32_t r_nan = cxpect_math_float_is_nan(iright);

	const uint32_t l_inf = cxpect_math_float_is_inf(ileft);
	const uint32_t r_inf = cxpect_math_float_is_inf(iright);

	const uint32_t any_nan = l_nan | r_nan;
	const uint32_t any_inf = l_inf | r_inf;

	const uint32_t safe_mask = (any_nan | any_inf) ? 0x0u : 0xffffffffu;

	cxpect_math_mask_32 s_left = {.u = ileft.u & safe_mask};
	cxpect_math_mask_32 s_right = {.u = iright.u & safe_mask};
	const float diff = cxpect_math_float_abs(s_left.f - s_right.f);

	#if CXPECT_MATCH_NAN_EQUALS_NAN
	const uint32_t nan_match = l_nan & r_nan;
	#else
	const uint32_t nan_match = 0;
	#endif

	const uint32_t dist_ok = (dist <= 1);
	const uint32_t eps_ok = (diff <= cxpect_math_comparison_eps5_bits.f);
	const uint32_t normal_match = eps_ok | ((!diff_sign) & dist_ok);

	return (both_zero | nan_match | ((!any_nan) & ((any_inf & exact) | ((!any_inf) & normal_match)))) != 0u;
}

	#define cxpect_math_double_exponent_mask 0x7FF0000000000000ULL
	#define cxpect_math_double_mantissa_mask 0x000FFFFFFFFFFFFFULL
	#define cxpect_math_double_sign_mask	 0x8000000000000000ULL

	#define cxpect_math_double_pos_nan ((cxpect_math_mask_64) {.u = 0x7FFFFFFFFFFFFFFFULL})
	#define cxpect_math_double_neg_nan ((cxpect_math_mask_64) {.u = 0xFFFFFFFFFFFFFFFFULL})
	#define cxpect_math_double_pos_inf ((cxpect_math_mask_64) {.u = 0x7FF0000000000000ULL})
	#define cxpect_math_double_neg_inf ((cxpect_math_mask_64) {.u = 0xFFF0000000000000ULL})

	#ifndef cxpect_math_double_eps_bits
		#define cxpect_math_double_eps_bits ((cxpect_math_mask_64) {.d = 1e-12})
	#endif
	#ifndef cxpect_math_double_comparison_eps9_bits
		#define cxpect_math_double_comparison_eps9_bits ((cxpect_math_mask_64) {.d = 1e-9})
	#endif

static inline uint64_t
cxpect_math_double_almost_zero(cxpect_math_mask_64 self)
{
	return ((self.u & (cxpect_math_double_exponent_mask | cxpect_math_double_mantissa_mask)) == 0) ||
		   (cxpect_math_double_abs(self.d) <= cxpect_math_double_eps_bits.d);
}

static inline uint64_t
cxpect_math_double_ulp_distance(const cxpect_math_mask_64 ileft, const cxpect_math_mask_64 iright)
{
	return (ileft.u > iright.u ? ileft.u : iright.u) - (ileft.u > iright.u ? iright.u : ileft.u);
}

static inline uint64_t
cxpect_math_double_is_nan(cxpect_math_mask_64 self)
{
	return (self.u & 0x7fffffffffffffffULL) > 0x7ff0000000000000ULL;
}

static inline uint64_t
cxpect_math_double_is_inf(cxpect_math_mask_64 self)
{
	return (self.u & 0x7fffffffffffffffULL) == 0x7ff0000000000000ULL;
}

static inline bool
cxpect_math_double_exact_abs_match(cxpect_math_mask_64 left, cxpect_math_mask_64 right)
{
	const uint64_t abs_mask = cxpect_math_double_exponent_mask | cxpect_math_double_mantissa_mask;
	return (left.u & abs_mask) == (right.u & abs_mask);
}

static inline bool
cxpect_math_double_equals(const double left, const double right)
{
	const cxpect_math_mask_64 ileft = {.d = left};
	const cxpect_math_mask_64 iright = {.d = right};

	const uint64_t diff_sign = (ileft.u ^ iright.u) >> 63;
	const uint64_t both_zero =
		(uint64_t) cxpect_math_double_almost_zero(ileft) & (uint64_t) cxpect_math_double_almost_zero(iright);
	const uint64_t exact = (ileft.u == iright.u);
	const uint64_t dist = cxpect_math_double_ulp_distance(ileft, iright);

	const uint64_t l_nan = cxpect_math_double_is_nan(ileft);
	const uint64_t r_nan = cxpect_math_double_is_nan(iright);

	const uint64_t l_inf = cxpect_math_double_is_inf(ileft);
	const uint64_t r_inf = cxpect_math_double_is_inf(iright);

	const uint64_t any_nan = l_nan | r_nan;
	const uint64_t any_inf = l_inf | r_inf;

	const uint64_t safe_mask = (any_nan | any_inf) ? 0x0ull : 0xffffffffffffffffull;

	cxpect_math_mask_64 s_left = {.u = ileft.u & safe_mask};
	cxpect_math_mask_64 s_right = {.u = iright.u & safe_mask};

	const double diff = cxpect_math_double_abs(s_left.d - s_right.d);

	#if CXPECT_MATCH_NAN_EQUALS_NAN
	const uint64_t nan_match = l_nan & r_nan;
	#else
	const uint64_t nan_match = 0;
	#endif

	const uint64_t dist_ok = (dist <= 1);
	const uint64_t eps_ok = (diff <= cxpect_math_double_comparison_eps9_bits.d);
	const uint64_t normal_match = eps_ok | ((!diff_sign) & dist_ok);

	return (both_zero | nan_match | ((!any_nan) & ((any_inf & exact) | ((!any_inf) & normal_match)))) != 0ul;
}
#else
static inline bool
cxpect_math_float_equals(float left, float right)
{
	const uint32_t exact = (uint32_t) (left <= right) & (uint32_t) (left >= right);
	const uint32_t opposite_signs = (uint32_t) (left < 0.0f) ^ (uint32_t) (right < 0.0f);

	const uint32_t near_zero = (uint32_t) (cxpect_math_float_abs(left) <= CXPECT_FLOAT_ABS_EPSILON) &
							   (uint32_t) (cxpect_math_float_abs(right) <= CXPECT_FLOAT_ABS_EPSILON);

	const uint32_t compare_delta = (exact | opposite_signs) ^ 1u;
	const float delta_left = compare_delta != 0u ? left : 0.0f;
	const float delta_right = compare_delta != 0u ? right : 0.0f;
	const uint32_t near_delta =
		(uint32_t) (cxpect_math_float_abs(delta_left - delta_right) <= CXPECT_FLOAT_ABS_EPSILON);

	return (exact | (opposite_signs & near_zero) | (compare_delta & near_delta)) != 0u;
}

static inline bool
cxpect_math_double_equals(double left, double right)
{
	const uint32_t exact = (uint32_t) (left <= right) & (uint32_t) (left >= right);
	const uint32_t opposite_signs = (uint32_t) (left < 0.0) ^ (uint32_t) (right < 0.0);

	const uint32_t near_zero = (uint32_t) (cxpect_math_double_abs(left) <= CXPECT_DOUBLE_ABS_EPSILON) &
							   (uint32_t) (cxpect_math_double_abs(right) <= CXPECT_DOUBLE_ABS_EPSILON);

	const uint32_t compare_delta = (exact | opposite_signs) ^ 1u;
	const double delta_left = compare_delta != 0u ? left : 0.0;
	const double delta_right = compare_delta != 0u ? right : 0.0;
	const uint32_t near_delta =
		(uint32_t) (cxpect_math_double_abs(delta_left - delta_right) <= CXPECT_DOUBLE_ABS_EPSILON);

	return (exact | (opposite_signs & near_zero) | (compare_delta & near_delta)) != 0u;
}
#endif

static inline bool
cxpect_math_long_double_equals(long double left, long double right)
{
	const bool exact = (left <= right) & (left >= right);

	const bool opposite_signs = (left < 0.0L) ^ (right < 0.0L);

	const bool near_zero = (uint64_t) (cxpect_math_long_double_abs(left) <= CXPECT_LONG_DOUBLE_ABS_EPSILON) &
						   (uint64_t) (cxpect_math_long_double_abs(right) <= CXPECT_LONG_DOUBLE_ABS_EPSILON);

	const bool near_delta = cxpect_math_long_double_abs(left - right) <= CXPECT_LONG_DOUBLE_ABS_EPSILON;

	return exact | (opposite_signs & near_zero) | (!opposite_signs & near_delta);
}

#if defined(__clang__)
	#pragma clang diagnostic pop
#elif defined(__GNUC__)
	#pragma GCC diagnostic pop
#endif

static inline int
cxpect_safe_strcmp_int(const void* left, const void* right)
{
	if (left == right)
	{
		return 0;
	}
	if (!left)
	{
		return -1;
	}
	if (!right)
	{
		return 1;
	}
	return strcmp((const char*) left, (const char*) right);
}

// comparison codegen

#define CXPECT_DEFINE_ORDERED_COMPARATORS(suffix_, type_)                                                              \
	static inline bool cxpect_compare_eq_##suffix_(type_ left, type_ right)                                            \
	{                                                                                                                  \
		return left == right;                                                                                          \
	}                                                                                                                  \
                                                                                                                       \
	static inline bool cxpect_compare_neq_##suffix_(type_ left, type_ right)                                           \
	{                                                                                                                  \
		return left != right;                                                                                          \
	}                                                                                                                  \
                                                                                                                       \
	static inline bool cxpect_compare_gt_##suffix_(type_ left, type_ right)                                            \
	{                                                                                                                  \
		return left > right;                                                                                           \
	}                                                                                                                  \
                                                                                                                       \
	static inline bool cxpect_compare_lt_##suffix_(type_ left, type_ right)                                            \
	{                                                                                                                  \
		return left < right;                                                                                           \
	}                                                                                                                  \
                                                                                                                       \
	static inline bool cxpect_compare_ge_##suffix_(type_ left, type_ right)                                            \
	{                                                                                                                  \
		return left >= right;                                                                                          \
	}                                                                                                                  \
                                                                                                                       \
	static inline bool cxpect_compare_le_##suffix_(type_ left, type_ right)                                            \
	{                                                                                                                  \
		return left <= right;                                                                                          \
	}

CXPECT_DEFINE_ORDERED_COMPARATORS(char, char)
CXPECT_DEFINE_ORDERED_COMPARATORS(schar, signed char)
CXPECT_DEFINE_ORDERED_COMPARATORS(uchar, unsigned char)
CXPECT_DEFINE_ORDERED_COMPARATORS(short, short)
CXPECT_DEFINE_ORDERED_COMPARATORS(ushort, unsigned short)
CXPECT_DEFINE_ORDERED_COMPARATORS(int, int)
CXPECT_DEFINE_ORDERED_COMPARATORS(uint, unsigned int)
CXPECT_DEFINE_ORDERED_COMPARATORS(long, long)
CXPECT_DEFINE_ORDERED_COMPARATORS(ulong, unsigned long)
CXPECT_DEFINE_ORDERED_COMPARATORS(llong, long long)
CXPECT_DEFINE_ORDERED_COMPARATORS(ullong, unsigned long long)
CXPECT_DEFINE_ORDERED_COMPARATORS(bool, bool)

#undef CXPECT_DEFINE_ORDERED_COMPARATORS

// comparison dispatch

static inline bool
cxpect_compare_eq_float(float left, float right)
{
	return cxpect_math_float_equals(left, right);
}

static inline bool
cxpect_compare_neq_float(float left, float right)
{
	return !cxpect_math_float_equals(left, right);
}

static inline bool
cxpect_compare_gt_float(float left, float right)
{
	return left > right;
}

static inline bool
cxpect_compare_lt_float(float left, float right)
{
	return left < right;
}

static inline bool
cxpect_compare_ge_float(float left, float right)
{
	return left >= right;
}

static inline bool
cxpect_compare_le_float(float left, float right)
{
	return left <= right;
}

static inline bool
cxpect_compare_eq_double(double left, double right)
{
	return cxpect_math_double_equals(left, right);
}

static inline bool
cxpect_compare_neq_double(double left, double right)
{
	return !cxpect_math_double_equals(left, right);
}

static inline bool
cxpect_compare_gt_double(double left, double right)
{
	return left > right;
}

static inline bool
cxpect_compare_lt_double(double left, double right)
{
	return left < right;
}

static inline bool
cxpect_compare_ge_double(double left, double right)
{
	return left >= right;
}

static inline bool
cxpect_compare_le_double(double left, double right)
{
	return left <= right;
}

static inline bool
cxpect_compare_eq_ldouble(long double left, long double right)
{
	return cxpect_math_long_double_equals(left, right);
}

static inline bool
cxpect_compare_neq_ldouble(long double left, long double right)
{
	return !cxpect_math_long_double_equals(left, right);
}

static inline bool
cxpect_compare_gt_ldouble(long double left, long double right)
{
	return left > right;
}

static inline bool
cxpect_compare_lt_ldouble(long double left, long double right)
{
	return left < right;
}

static inline bool
cxpect_compare_ge_ldouble(long double left, long double right)
{
	return left >= right;
}

static inline bool
cxpect_compare_le_ldouble(long double left, long double right)
{
	return left <= right;
}

static inline bool
cxpect_compare_eq_cstr(const char* left, const char* right)
{
	return cxpect_safe_strcmp_int(left, right) == 0;
}

static inline bool
cxpect_compare_neq_cstr(const char* left, const char* right)
{
	return cxpect_safe_strcmp_int(left, right) != 0;
}

static inline bool
cxpect_compare_gt_cstr(const char* left, const char* right)
{
	return cxpect_safe_strcmp_int(left, right) > 0;
}

static inline bool
cxpect_compare_lt_cstr(const char* left, const char* right)
{
	return cxpect_safe_strcmp_int(left, right) < 0;
}

static inline bool
cxpect_compare_ge_cstr(const char* left, const char* right)
{
	return cxpect_safe_strcmp_int(left, right) >= 0;
}

static inline bool
cxpect_compare_le_cstr(const char* left, const char* right)
{
	return cxpect_safe_strcmp_int(left, right) <= 0;
}

static inline bool
cxpect_compare_eq_ptr(const void* left, const void* right)
{
	return left == right;
}

static inline bool
cxpect_compare_neq_ptr(const void* left, const void* right)
{
	return left != right;
}

static inline bool
cxpect_compare_order_unsupported(long double left, long double right)
{
	(void) left;
	(void) right;
	return false;
}

#ifndef CXPECT_CUSTOM_EQ
	#define CXPECT_CUSTOM_EQ
#endif
#ifndef CXPECT_CUSTOM_NEQ
	#define CXPECT_CUSTOM_NEQ
#endif
#ifndef CXPECT_CUSTOM_GT
	#define CXPECT_CUSTOM_GT
#endif
#ifndef CXPECT_CUSTOM_LT
	#define CXPECT_CUSTOM_LT
#endif
#ifndef CXPECT_CUSTOM_GE
	#define CXPECT_CUSTOM_GE
#endif
#ifndef CXPECT_CUSTOM_LE
	#define CXPECT_CUSTOM_LE
#endif

#define cxpect_common_type_expr(left_, right_) (false ? (left_) : (right_))

// ==
#define cxpect_is_eq(left_, right_)                                                                                    \
	_Generic(cxpect_common_type_expr((left_), (right_)),                                                               \
		char: cxpect_compare_eq_char,                                                                                  \
		signed char: cxpect_compare_eq_schar,                                                                          \
		unsigned char: cxpect_compare_eq_uchar,                                                                        \
		short: cxpect_compare_eq_short,                                                                                \
		unsigned short: cxpect_compare_eq_ushort,                                                                      \
		int: cxpect_compare_eq_int,                                                                                    \
		unsigned int: cxpect_compare_eq_uint,                                                                          \
		long: cxpect_compare_eq_long,                                                                                  \
		unsigned long: cxpect_compare_eq_ulong,                                                                        \
		long long: cxpect_compare_eq_llong,                                                                            \
		unsigned long long: cxpect_compare_eq_ullong,                                                                  \
		float: cxpect_compare_eq_float,                                                                                \
		double: cxpect_compare_eq_double,                                                                              \
		long double: cxpect_compare_eq_ldouble,                                                                        \
		char*: cxpect_compare_eq_cstr,                                                                                 \
		const char*: cxpect_compare_eq_cstr,                                                                           \
		bool: cxpect_compare_eq_bool,                                                                                  \
		CXPECT_CUSTOM_EQ default: cxpect_compare_eq_ptr)((left_), (right_))

// !=
#define cxpect_is_neq(left_, right_)                                                                                   \
	_Generic(cxpect_common_type_expr((left_), (right_)),                                                               \
		char: cxpect_compare_neq_char,                                                                                 \
		signed char: cxpect_compare_neq_schar,                                                                         \
		unsigned char: cxpect_compare_neq_uchar,                                                                       \
		short: cxpect_compare_neq_short,                                                                               \
		unsigned short: cxpect_compare_neq_ushort,                                                                     \
		int: cxpect_compare_neq_int,                                                                                   \
		unsigned int: cxpect_compare_neq_uint,                                                                         \
		long: cxpect_compare_neq_long,                                                                                 \
		unsigned long: cxpect_compare_neq_ulong,                                                                       \
		long long: cxpect_compare_neq_llong,                                                                           \
		unsigned long long: cxpect_compare_neq_ullong,                                                                 \
		float: cxpect_compare_neq_float,                                                                               \
		double: cxpect_compare_neq_double,                                                                             \
		long double: cxpect_compare_neq_ldouble,                                                                       \
		char*: cxpect_compare_neq_cstr,                                                                                \
		const char*: cxpect_compare_neq_cstr,                                                                          \
		bool: cxpect_compare_neq_bool,                                                                                 \
		CXPECT_CUSTOM_NEQ default: cxpect_compare_neq_ptr)((left_), (right_))

// >
#define cxpect_is_gt(left_, right_)                                                                                    \
	_Generic(cxpect_common_type_expr((left_), (right_)),                                                               \
		char: cxpect_compare_gt_char,                                                                                  \
		signed char: cxpect_compare_gt_schar,                                                                          \
		unsigned char: cxpect_compare_gt_uchar,                                                                        \
		short: cxpect_compare_gt_short,                                                                                \
		unsigned short: cxpect_compare_gt_ushort,                                                                      \
		int: cxpect_compare_gt_int,                                                                                    \
		unsigned int: cxpect_compare_gt_uint,                                                                          \
		long: cxpect_compare_gt_long,                                                                                  \
		unsigned long: cxpect_compare_gt_ulong,                                                                        \
		long long: cxpect_compare_gt_llong,                                                                            \
		unsigned long long: cxpect_compare_gt_ullong,                                                                  \
		float: cxpect_compare_gt_float,                                                                                \
		double: cxpect_compare_gt_double,                                                                              \
		long double: cxpect_compare_gt_ldouble,                                                                        \
		char*: cxpect_compare_gt_cstr,                                                                                 \
		const char*: cxpect_compare_gt_cstr,                                                                           \
		bool: cxpect_compare_gt_bool,                                                                                  \
		CXPECT_CUSTOM_GT default: cxpect_compare_order_unsupported)((left_), (right_))

// <
#define cxpect_is_lt(left_, right_)                                                                                    \
	_Generic(cxpect_common_type_expr((left_), (right_)),                                                               \
		char: cxpect_compare_lt_char,                                                                                  \
		signed char: cxpect_compare_lt_schar,                                                                          \
		unsigned char: cxpect_compare_lt_uchar,                                                                        \
		short: cxpect_compare_lt_short,                                                                                \
		unsigned short: cxpect_compare_lt_ushort,                                                                      \
		int: cxpect_compare_lt_int,                                                                                    \
		unsigned int: cxpect_compare_lt_uint,                                                                          \
		long: cxpect_compare_lt_long,                                                                                  \
		unsigned long: cxpect_compare_lt_ulong,                                                                        \
		long long: cxpect_compare_lt_llong,                                                                            \
		unsigned long long: cxpect_compare_lt_ullong,                                                                  \
		float: cxpect_compare_lt_float,                                                                                \
		double: cxpect_compare_lt_double,                                                                              \
		long double: cxpect_compare_lt_ldouble,                                                                        \
		char*: cxpect_compare_lt_cstr,                                                                                 \
		const char*: cxpect_compare_lt_cstr,                                                                           \
		bool: cxpect_compare_lt_bool,                                                                                  \
		CXPECT_CUSTOM_LT default: cxpect_compare_order_unsupported)((left_), (right_))

// >=
#define cxpect_is_ge(left_, right_)                                                                                    \
	_Generic(cxpect_common_type_expr((left_), (right_)),                                                               \
		char: cxpect_compare_ge_char,                                                                                  \
		signed char: cxpect_compare_ge_schar,                                                                          \
		unsigned char: cxpect_compare_ge_uchar,                                                                        \
		short: cxpect_compare_ge_short,                                                                                \
		unsigned short: cxpect_compare_ge_ushort,                                                                      \
		int: cxpect_compare_ge_int,                                                                                    \
		unsigned int: cxpect_compare_ge_uint,                                                                          \
		long: cxpect_compare_ge_long,                                                                                  \
		unsigned long: cxpect_compare_ge_ulong,                                                                        \
		long long: cxpect_compare_ge_llong,                                                                            \
		unsigned long long: cxpect_compare_ge_ullong,                                                                  \
		float: cxpect_compare_ge_float,                                                                                \
		double: cxpect_compare_ge_double,                                                                              \
		long double: cxpect_compare_ge_ldouble,                                                                        \
		char*: cxpect_compare_ge_cstr,                                                                                 \
		const char*: cxpect_compare_ge_cstr,                                                                           \
		bool: cxpect_compare_ge_bool,                                                                                  \
		CXPECT_CUSTOM_GE default: cxpect_compare_order_unsupported)((left_), (right_))

// <=
#define cxpect_is_le(left_, right_)                                                                                    \
	_Generic(cxpect_common_type_expr((left_), (right_)),                                                               \
		char: cxpect_compare_le_char,                                                                                  \
		signed char: cxpect_compare_le_schar,                                                                          \
		unsigned char: cxpect_compare_le_uchar,                                                                        \
		short: cxpect_compare_le_short,                                                                                \
		unsigned short: cxpect_compare_le_ushort,                                                                      \
		int: cxpect_compare_le_int,                                                                                    \
		unsigned int: cxpect_compare_le_uint,                                                                          \
		long: cxpect_compare_le_long,                                                                                  \
		unsigned long: cxpect_compare_le_ulong,                                                                        \
		long long: cxpect_compare_le_llong,                                                                            \
		unsigned long long: cxpect_compare_le_ullong,                                                                  \
		float: cxpect_compare_le_float,                                                                                \
		double: cxpect_compare_le_double,                                                                              \
		long double: cxpect_compare_le_ldouble,                                                                        \
		char*: cxpect_compare_le_cstr,                                                                                 \
		const char*: cxpect_compare_le_cstr,                                                                           \
		bool: cxpect_compare_le_bool,                                                                                  \
		CXPECT_CUSTOM_LE default: cxpect_compare_order_unsupported)((left_), (right_))

#define cxpect_unary_impl_1(expr, expected_bool) cxpect_expect_unary_base(expr, expected_bool, NULL)

#define cxpect_unary_impl_0(expr, expected_bool, msg) cxpect_expect_unary_base(expr, expected_bool, msg)

#define cxpect_expect_op_base(actual_expr_, expected_expr_, op_name_, op_str_, msg_)                                   \
	do                                                                                                                 \
	{                                                                                                                  \
		cxpect_typeof(1 ? (actual_expr_) : (actual_expr_)) _a = (actual_expr_);                                        \
		cxpect_typeof(1 ? (expected_expr_) : (expected_expr_)) _e = (expected_expr_);                                  \
                                                                                                                       \
		const char* const cxpect_internal_message = (msg_);                                                            \
		const bool cxpect_internal_result = cxpect_is_##op_name_(_a, _e);                                              \
                                                                                                                       \
		if (cxpect_unlikely(!cxpect_internal_result))                                                                  \
		{                                                                                                              \
			fprintf(stderr,                                                                                            \
				"\n%s[Test Failed]%s at %s:%d\n",                                                                      \
				cxpect_get_terminal_color(CxpectTerminalColorsRed),                                                    \
				cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                  \
				__FILE__,                                                                                              \
				__LINE__);                                                                                             \
			if (cxpect_internal_message)                                                                               \
			{                                                                                                          \
				fprintf(stderr,                                                                                        \
					"  %sMessage:%s   %s\n",                                                                           \
					cxpect_get_terminal_color(CxpectTerminalColorsBrown),                                              \
					cxpect_get_terminal_color(CxpectTerminalColorsClear),                                              \
					(const char*) cxpect_internal_message);                                                            \
			}                                                                                                          \
			fprintf(stderr,                                                                                            \
				"  %sCondition:%s %s %s %s\n",                                                                         \
				cxpect_get_terminal_color(CxpectTerminalColorsYellow),                                                 \
				cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                  \
				#actual_expr_,                                                                                         \
				op_str_,                                                                                               \
				#expected_expr_);                                                                                      \
			fprintf(stderr,                                                                                            \
				"  %sEvaluates:%s ",                                                                                   \
				cxpect_get_terminal_color(CxpectTerminalColorsYellow),                                                 \
				cxpect_get_terminal_color(CxpectTerminalColorsClear));                                                 \
			cxpect_print_generic_val(stderr, _a);                                                                      \
			fprintf(stderr, " %s ", op_str_);                                                                          \
			cxpect_print_generic_val(stderr, _e);                                                                      \
			fprintf(stderr, "\n\n");                                                                                   \
			cxpect_fail;                                                                                               \
		}                                                                                                              \
	} while (0)

#define cxpect_expect_unary_base(expr_, expected_bool_, msg_)                                                          \
	do                                                                                                                 \
	{                                                                                                                  \
		bool cxpect_internal_actual_bool = !!(expr_);                                                                  \
		const bool cxpect_internal_expected_bool = (expected_bool_);                                                   \
		const char* const cxpect_internal_message = (msg_);                                                            \
		if (cxpect_internal_actual_bool != (cxpect_internal_expected_bool))                                            \
		{                                                                                                              \
			fprintf(stderr,                                                                                            \
				"\n%s[Test Failed]%s at %s:%d\n",                                                                      \
				cxpect_get_terminal_color(CxpectTerminalColorsRed),                                                    \
				cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                  \
				__FILE__,                                                                                              \
				__LINE__);                                                                                             \
			if (msg_)                                                                                                  \
			{                                                                                                          \
				fprintf(stderr,                                                                                        \
					"  %sMessage:%s   %s\n",                                                                           \
					cxpect_get_terminal_color(CxpectTerminalColorsBrown),                                              \
					cxpect_get_terminal_color(CxpectTerminalColorsClear),                                              \
					cxpect_internal_message);                                                                          \
			}                                                                                                          \
			fprintf(stderr,                                                                                            \
				"  %sCondition:%s %s\n",                                                                               \
				cxpect_get_terminal_color(CxpectTerminalColorsYellow),                                                 \
				cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                  \
				#expr_);                                                                                               \
			fprintf(stderr,                                                                                            \
				"  %sExpected:%s  %s\n",                                                                               \
				cxpect_get_terminal_color(CxpectTerminalColorsYellow),                                                 \
				cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                  \
				cxpect_internal_expected_bool ? "true" : "false");                                                     \
			fprintf(stderr,                                                                                            \
				"  %sActual:%s    %s\n\n",                                                                             \
				cxpect_get_terminal_color(CxpectTerminalColorsYellow),                                                 \
				cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                  \
				cxpect_internal_actual_bool ? "true" : "false");                                                       \
			cxpect_fail;                                                                                               \
		}                                                                                                              \
	} while (0)

#define cxpect_expect_eq_2(actual_, expected_)			  cxpect_expect_op_base(actual_, expected_, eq, "==", NULL)
#define cxpect_expect_eq_3(actual_, expected_, message_)  cxpect_expect_op_base(actual_, expected_, eq, "==", message_)
#define cxpect_expect_neq_2(actual_, expected_)			  cxpect_expect_op_base(actual_, expected_, neq, "!=", NULL)
#define cxpect_expect_neq_3(actual_, expected_, message_) cxpect_expect_op_base(actual_, expected_, neq, "!=", message_)
#define cxpect_expect_gt_2(actual_, expected_)			  cxpect_expect_op_base(actual_, expected_, gt, ">", NULL)
#define cxpect_expect_gt_3(actual_, expected_, message_)  cxpect_expect_op_base(actual_, expected_, gt, ">", message_)
#define cxpect_expect_lt_2(actual_, expected_)			  cxpect_expect_op_base(actual_, expected_, lt, "<", NULL)
#define cxpect_expect_lt_3(actual_, expected_, message_)  cxpect_expect_op_base(actual_, expected_, lt, "<", message_)
#define cxpect_expect_ge_2(actual_, expected_)			  cxpect_expect_op_base(actual_, expected_, ge, ">=", NULL)
#define cxpect_expect_ge_3(actual_, expected_, message_)  cxpect_expect_op_base(actual_, expected_, ge, ">=", message_)
#define cxpect_expect_le_2(actual_, expected_)			  cxpect_expect_op_base(actual_, expected_, le, "<=", NULL)
#define cxpect_expect_le_3(actual_, expected_, message_)  cxpect_expect_op_base(actual_, expected_, le, "<=", message_)
#define cxpect_expect_true_1(expr_)						  cxpect_expect_unary_base(expr_, true, NULL)
#define cxpect_expect_true_2(expr_, message_)			  cxpect_expect_unary_base(expr_, true, message_)
#define cxpect_expect_false_1(expr_)					  cxpect_expect_unary_base(expr_, false, NULL)
#define cxpect_expect_false_2(expr_, message_)			  cxpect_expect_unary_base(expr_, false, message_)

#define cxpect_expect_eq(...)  cxpect_macro_concat(cxpect_expect_eq_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)
#define cxpect_expect_neq(...) cxpect_macro_concat(cxpect_expect_neq_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)
#define cxpect_expect_gt(...)  cxpect_macro_concat(cxpect_expect_gt_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)
#define cxpect_expect_lt(...)  cxpect_macro_concat(cxpect_expect_lt_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)
#define cxpect_expect_ge(...)  cxpect_macro_concat(cxpect_expect_ge_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)
#define cxpect_expect_le(...)  cxpect_macro_concat(cxpect_expect_le_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)
#define cxpect_expect_true(...)                                                                                        \
	cxpect_macro_concat(cxpect_expect_true_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)
#define cxpect_expect_false(...)                                                                                       \
	cxpect_macro_concat(cxpect_expect_false_, cxpect_macro_count_arg(__VA_ARGS__))(__VA_ARGS__)

// match API

typedef struct cxpect_match_state
{
	bool done;
	bool once;
} cxpect_match_state_t;

static inline bool
cxpect_match_take(bool* done, bool condition)
{
	if (*done || !condition)
	{
		return false;
	}
	*done = true;
	return true;
}

static inline void
cxpect_report_match_fallthrough(const char* file, int line, const char* expression)
{
	fflush(stdout);
	fprintf(stderr,
		"\n%s[Match Failed]%s at %s:%d\n",
		cxpect_get_terminal_color(CxpectTerminalColorsRed),
		cxpect_get_terminal_color(CxpectTerminalColorsClear),
		file,
		line);
	fprintf(stderr,
		"  %sReason:%s   Unhandled value (fallthrough)\n",
		cxpect_get_terminal_color(CxpectTerminalColorsYellow),
		cxpect_get_terminal_color(CxpectTerminalColorsClear));
	fprintf(stderr,
		"  %sSubject:%s  %s\n\n",
		cxpect_get_terminal_color(CxpectTerminalColorsYellow),
		cxpect_get_terminal_color(CxpectTerminalColorsClear),
		expression);
}

#define cxpect_match_fallthrough(file_, line_, expression_)                                                            \
	(cxpect_report_match_fallthrough((file_), (line_), (expression_)), cxpect_fail)

static inline void
cxpect_finish_match(cxpect_ctx_t* ctx, bool done, const char* file, int line, const char* expression)
{
	if (done)
	{
		return;
	}
	cxpect_report_match_fallthrough(file, line, expression);
	if (ctx != NULL && ctx->fault_env != NULL)
	{
		longjmp(*ctx->fault_env, 1);
	}
	cxpect_fail_impl;
}

#define cxpect_expect_match(subject_)                                                                                  \
	for (cxpect_match_state_t cxpect_internal_match_state = {false, true}; cxpect_internal_match_state.once;           \
		cxpect_internal_match_state.once = false,                                                                      \
							  cxpect_finish_match(cxpect_default_ctx,                                                  \
								  cxpect_internal_match_state.done,                                                    \
								  __FILE__,                                                                            \
								  __LINE__,                                                                            \
								  #subject_))                                                                          \
		for (cxpect_typeof(false ? (subject_) : (subject_))                                                            \
				 cxpect_internal_match_subject = (subject_),                                                           \
				 *cxpect_internal_match_subject_once = &cxpect_internal_match_subject;                                 \
			((void) cxpect_internal_match_subject, cxpect_internal_match_subject_once != NULL);                        \
			cxpect_internal_match_subject_once = NULL)

#define cxpect_expect_match_as(subject_, name_)                                                                        \
	for (cxpect_match_state_t cxpect_internal_match_state = {false, true}; cxpect_internal_match_state.once;           \
		cxpect_internal_match_state.once = false,                                                                      \
							  cxpect_finish_match(cxpect_default_ctx,                                                  \
								  cxpect_internal_match_state.done,                                                    \
								  __FILE__,                                                                            \
								  __LINE__,                                                                            \
								  #subject_))                                                                          \
		for (cxpect_typeof(false ? (subject_) : (subject_)) name_ = (subject_),                                        \
															cxpect_internal_match_subject = name_,                     \
															*cxpect_internal_match_subject_once = &(name_);            \
			((void) cxpect_internal_match_subject, cxpect_internal_match_subject_once != NULL);                        \
			cxpect_internal_match_subject_once = NULL)

#define cxpect_fail_match(message_)                                                                                    \
	do                                                                                                                 \
	{                                                                                                                  \
		const char* const cxpect_internal_match_message = (message_);                                                  \
		fflush(stdout);                                                                                                \
		fprintf(stderr,                                                                                                \
			"\n%s[Match Failed]%s at %s:%d\n",                                                                         \
			cxpect_get_terminal_color(CxpectTerminalColorsRed),                                                        \
			cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                      \
			__FILE__,                                                                                                  \
			__LINE__);                                                                                                 \
		fprintf(stderr,                                                                                                \
			"  %sReason:%s   %s\n\n",                                                                                  \
			cxpect_get_terminal_color(CxpectTerminalColorsYellow),                                                     \
			cxpect_get_terminal_color(CxpectTerminalColorsClear),                                                      \
			cxpect_internal_match_message != NULL ? cxpect_internal_match_message : "(no message)");                   \
		cxpect_fail;                                                                                                   \
	} while (false)

#define cxpect_when_let(condition_, subject_, field_, variable_)                                                       \
	if (cxpect_match_take(&cxpect_internal_match_state.done, !!(condition_)))                                          \
		for (cxpect_typeof(false ? (subject_).field_ : (subject_).field_) variable_ = (subject_).field_,               \
																		  *cxpect_internal_let_once = &(variable_);    \
			cxpect_internal_let_once != NULL;                                                                          \
			cxpect_internal_let_once = NULL)

// random API

#ifdef CXPECT_ENABLE_RANDOM
void
cxpect_rnd_init(struct cxpect_ctx*);

int8_t
cxpect_rnd_next_i8(struct cxpect_ctx* ctx);
uint8_t
cxpect_rnd_next_u8(struct cxpect_ctx* ctx);
int16_t
cxpect_rnd_next_i16(struct cxpect_ctx* ctx);
uint16_t
cxpect_rnd_next_u16(struct cxpect_ctx* ctx);
int32_t
cxpect_rnd_next_i32(struct cxpect_ctx* ctx);
uint32_t
cxpect_rnd_next_u32(struct cxpect_ctx* ctx);
int64_t
cxpect_rnd_next_i64(struct cxpect_ctx* ctx);
uint64_t
cxpect_rnd_next_u64(struct cxpect_ctx* ctx);
float
cxpect_rnd_next_float(struct cxpect_ctx*);
double
cxpect_rnd_next_double(struct cxpect_ctx* ctx);

int8_t
cxpect_rnd_range_i8(cxpect_ctx_t* ctx, int8_t from, int8_t to);
uint8_t
cxpect_rnd_range_u8(cxpect_ctx_t* ctx, uint8_t from, uint8_t to);
int16_t
cxpect_rnd_range_i16(cxpect_ctx_t* ctx, int16_t from, int16_t to);
uint16_t
cxpect_rnd_range_u16(cxpect_ctx_t* ctx, uint16_t from, uint16_t to);
int32_t
cxpect_rnd_range_i32(cxpect_ctx_t* ctx, int32_t from, int32_t to);
uint32_t
cxpect_rnd_range_u32(cxpect_ctx_t* ctx, uint32_t from, uint32_t to);
int64_t
cxpect_rnd_range_i64(cxpect_ctx_t* ctx, int64_t from, int64_t to);
uint64_t
cxpect_rnd_range_u64(cxpect_ctx_t* ctx, uint64_t from, uint64_t to);
float
cxpect_rnd_range_float(cxpect_ctx_t* ctx, float from, float to);
double
cxpect_rnd_range_double(cxpect_ctx_t* ctx, double from, double to);
#endif

#ifdef CXPECT_IMPLEMENTATION
void
cxpect_run_tests(cxpect_ctx_t* ctx, const cxpect_case_t* cases)
{
	if (ctx == NULL || cases == NULL)
	{
		cxpect_fail_impl;
	}
	for (const cxpect_case_t* current = cases; current->name != NULL; ++current)
	{
		if (current->fn == NULL)
		{
			cxpect_fail_impl;
		}
		if (cxpect_unlikely(ctx->filter != NULL && strstr(current->name, ctx->filter) == NULL))
		{
			continue;
		}
		fprintf(stdout,
			"\n%s▶ Running group: %s%s\n",
			cxpect_get_terminal_color(CxpectTerminalColorsCyan),
			cxpect_get_terminal_color(CxpectTerminalColorsClear),
			current->name);
		current->fn(ctx);
	}

	uint32_t total_tests = ctx->tests_passed + ctx->tests_failed;
	if (cxpect_unlikely(ctx->tests_failed > 0))
	{
		fprintf(stderr,
			"\n%s✖ %u of %u tests failed%s\n\n",
			cxpect_get_terminal_color(CxpectTerminalColorsRed),
			ctx->tests_failed,
			total_tests,
			cxpect_get_terminal_color(CxpectTerminalColorsClear));

	#ifdef CXPECT_ENABLE_RANDOM
		fprintf(stderr,
			"  %sReproduce with:%s CXPECT_SEED=%016llx:%016llx\n",
			cxpect_get_terminal_color(CxpectTerminalColorsCyan),
			cxpect_get_terminal_color(CxpectTerminalColorsClear),
			(unsigned long long) ctx->seed[0],
			(unsigned long long) ctx->seed[1]);
	#endif
		fputc('\n', stderr);
	}
	else
	{
		fprintf(stdout,
			"\n%s✓ All %u tests completed successfully%s\n\n",
			cxpect_get_terminal_color(CxpectTerminalColorsGreen),
			total_tests,
			cxpect_get_terminal_color(CxpectTerminalColorsClear));
	}
}

void
cxpect_ctx_post_init(cxpect_ctx_t* ctx)
{
	if (ctx == NULL)
	{
		cxpect_fail_impl;
	}

	#ifdef CXPECT_ENABLE_RANDOM
	cxpect_rnd_init(ctx);
	#endif
}

	#ifdef CXPECT_ENABLE_RANDOM
		#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
			#include <fcntl.h>
			#include <unistd.h>
static bool
cxpect_unix_entropy(uint8_t* destination, size_t count)
{
	int file = open("/dev/urandom", O_RDONLY);
	if (file < 0)
	{
		return false;
	}

	size_t offset = 0;
	while (offset < count)
	{
		const ssize_t amount = read(file, destination + offset, count - offset);
		if (amount > 0)
		{
			offset += (size_t) amount;
			continue;
		}
		if (amount < 0 && errno == EINTR)
		{
			continue;
		}
		close(file);
		return false;
	}

	close(file);
	return true;
}
		#endif

/*
 * PCG32 random number generator (XSH RR).
 *
 * Derived from:
 * https://github.com/imneme/pcg-c
 *
 * Copyright (c) 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Modified and integrated into cxpect by Iaroslav Borodkin, 2026.
 */
static inline uint32_t
cxpect_pcg_next(cxpect_pcg_state_t* pcg)
{
	uint64_t old_state = pcg->state;
	pcg->state = old_state * (uint64_t) {6364136223846793005} + pcg->inc;
	uint32_t xorshifted = (uint32_t) (((old_state >> 18u) ^ old_state) >> 27u);
	uint32_t rot = (uint32_t) (old_state >> 59u);
	return (xorshifted >> rot) | (xorshifted << ((0u - rot) & 31u));
}

static cxpect_pcg_state_t
cxpect_pcg_new(uint64_t seed0, uint64_t seed1)
{
	cxpect_pcg_state_t result = {
		.state = 0,
		.inc = ((seed1 & 0x7ffffffffffffffful) << 1u) | 1ul,
	};
	cxpect_pcg_next(&result);
	result.state += seed0;
	cxpect_pcg_next(&result);
	return result;
}

static int
cxpect_hex_digit_value(char character)
{
	if (character >= '0' && character <= '9')
	{
		return character - '0';
	}
	if (character >= 'a' && character <= 'f')
	{
		return character - 'a' + 10;
	}
	if (character >= 'A' && character <= 'F')
	{
		return character - 'A' + 10;
	}
	return -1;
}

static bool
cxpect_parse_hex_u64(const char** cursor, char terminator, uint64_t* result)
{
	const char* current = *cursor;
	uint64_t value = 0;
	bool has_digit = false;

	for (;;)
	{
		const int digit = cxpect_hex_digit_value(*current);
		if (digit < 0)
		{
			break;
		}
		has_digit = true;
		if (value > (UINT64_MAX - (uint64_t) digit) / UINT64_C(16))
		{
			return false;
		}
		value = value * UINT64_C(16) + (uint64_t) digit;
		++current;
	}

	if (!has_digit || *current != terminator)
	{
		return false;
	}

	*cursor = current + (terminator == '\0' ? 0 : 1);
	*result = value;
	return true;
}

static bool
cxpect_parse_seed(const char* text, uint64_t* seed)
{
	if (text == NULL)
	{
		return false;
	}

	const char* cursor = text;
	return cxpect_parse_hex_u64(&cursor, ':', &seed[0]) && cxpect_parse_hex_u64(&cursor, '\0', &seed[1]);
}

void
cxpect_rnd_init(cxpect_ctx_t* ctx)
{
	if (ctx == NULL)
	{
		cxpect_fail_impl;
	}

		#ifdef CXPECT_DETERMINISTIC_SEED
	const char* env_seed = CXPECT_DETERMINISTIC_SEED;
		#else
	const char* env_seed = getenv("CXPECT_SEED");
		#endif

	bool seed_loaded = cxpect_parse_seed(env_seed, ctx->seed);

		#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
	if (!seed_loaded)
	{
		seed_loaded = cxpect_unix_entropy((uint8_t*) ctx->seed, sizeof(ctx->seed));
	}
		#endif

	if (!seed_loaded)
	{
		const uint64_t wall_time = (uint64_t) time(NULL);
		const uint64_t cpu_time = (uint64_t) clock();
		ctx->seed[0] = wall_time ^ 0x9e3779b97f4a7c15ul;
		ctx->seed[1] = cpu_time ^ 0xda3e39cb94b95bdbul;
	}

	ctx->rng_state = cxpect_pcg_new(ctx->seed[0], ctx->seed[1]);
}

static inline int8_t
cxpect_i8_from_biased(uint8_t key)
{
	return key < UINT8_C(0x80) ? (int8_t) (INT8_MIN + (int16_t) key) : (int8_t) (key - UINT8_C(0x80));
}

static inline int16_t
cxpect_i16_from_biased(uint16_t key)
{
	return key < UINT16_C(0x8000) ? (int16_t) (INT16_MIN + (int32_t) key) : (int16_t) (key - UINT16_C(0x8000));
}

static inline int32_t
cxpect_i32_from_biased(uint32_t key)
{
	return key < 0x80000000u ? (int32_t) (INT32_MIN + (int64_t) key) : (int32_t) (key - 0x80000000u);
}

static inline int64_t
cxpect_i64_from_biased(uint64_t key)
{
	if (key < UINT64_C(0x8000000000000000))
	{
		return INT64_MIN + (int64_t) key;
	}
	return (int64_t) (key - UINT64_C(0x8000000000000000));
}

static inline uint32_t
cxpect_bounded_u32(cxpect_ctx_t* ctx, uint32_t bound)
{
	if (bound == 0u)
	{
		return cxpect_rnd_next_u32(ctx);
	}

	const uint32_t threshold = (0u - bound) % bound;
	for (;;)
	{
		const uint32_t value = cxpect_rnd_next_u32(ctx);
		if (value >= threshold)
		{
			return value % bound;
		}
	}
}

static inline uint64_t
cxpect_bounded_u64(cxpect_ctx_t* ctx, uint64_t bound)
{
	if (bound == 0u)
	{
		return cxpect_rnd_next_u64(ctx);
	}

	const uint64_t threshold = (UINT64_C(0) - bound) % bound;
	for (;;)
	{
		const uint64_t value = cxpect_rnd_next_u64(ctx);
		if (value >= threshold)
		{
			return value % bound;
		}
	}
}

int8_t
cxpect_rnd_next_i8(cxpect_ctx_t* ctx)
{
	return cxpect_i8_from_biased((uint8_t) cxpect_rnd_next_u32(ctx));
}

uint8_t
cxpect_rnd_next_u8(cxpect_ctx_t* ctx)
{
	return (uint8_t) (cxpect_rnd_next_u32(ctx) & 0xFFu);
}

int16_t
cxpect_rnd_next_i16(cxpect_ctx_t* ctx)
{
	return cxpect_i16_from_biased((uint16_t) cxpect_rnd_next_u32(ctx));
}

uint16_t
cxpect_rnd_next_u16(cxpect_ctx_t* ctx)
{
	return (uint16_t) (cxpect_rnd_next_u32(ctx) & 0xFFFFu);
}

int32_t
cxpect_rnd_next_i32(cxpect_ctx_t* ctx)
{
	return cxpect_i32_from_biased(cxpect_rnd_next_u32(ctx));
}

uint32_t
cxpect_rnd_next_u32(cxpect_ctx_t* ctx)
{
	return cxpect_pcg_next(&ctx->rng_state);
}

int64_t
cxpect_rnd_next_i64(cxpect_ctx_t* ctx)
{
	return cxpect_i64_from_biased(cxpect_rnd_next_u64(ctx));
}

uint64_t
cxpect_rnd_next_u64(cxpect_ctx_t* ctx)
{
	uint64_t high = (uint64_t) cxpect_rnd_next_u32(ctx);
	uint64_t low = (uint64_t) cxpect_rnd_next_u32(ctx);
	return (high << 32u) | low;
}

float
cxpect_rnd_next_float(cxpect_ctx_t* ctx)
{
	return (float) (cxpect_rnd_next_u32(ctx) >> 8u) * 0x1p-24f;
}

double
cxpect_rnd_next_double(cxpect_ctx_t* ctx)
{
	return (double) (cxpect_rnd_next_u64(ctx) >> 11u) * 0x1p-53;
}

int8_t
cxpect_rnd_range_i8(cxpect_ctx_t* ctx, int8_t from, int8_t to)
{
	if (from > to)
	{
		const int8_t tmp = from;
		from = to;
		to = tmp;
	}
	const uint8_t first = ((uint8_t) from) ^ UINT8_C(0x80);
	const uint8_t last = ((uint8_t) to) ^ UINT8_C(0x80);
	const uint32_t span = (uint32_t) last - (uint32_t) first + 1u;
	return cxpect_i8_from_biased((uint8_t) ((uint32_t) first + cxpect_bounded_u32(ctx, span)));
}

uint8_t
cxpect_rnd_range_u8(cxpect_ctx_t* ctx, uint8_t from, uint8_t to)
{
	if (from > to)
	{
		const uint8_t temp = from;
		from = to;
		to = temp;
	}
	const uint32_t span = (uint32_t) to - (uint32_t) from + 1u;
	return (uint8_t) ((uint32_t) from + cxpect_bounded_u32(ctx, span));
}

int16_t
cxpect_rnd_range_i16(cxpect_ctx_t* ctx, int16_t from, int16_t to)
{
	if (from > to)
	{
		const int16_t tmp = from;
		from = to;
		to = tmp;
	}
	const uint16_t first = ((uint16_t) from) ^ UINT16_C(0x8000);
	const uint16_t last = ((uint16_t) to) ^ UINT16_C(0x8000);
	const uint32_t span = (uint32_t) last - (uint32_t) first + 1u;
	return cxpect_i16_from_biased((uint16_t) ((uint32_t) first + cxpect_bounded_u32(ctx, span)));
}

uint16_t
cxpect_rnd_range_u16(cxpect_ctx_t* ctx, uint16_t from, uint16_t to)
{
	if (from > to)
	{
		const uint16_t tmp = from;
		from = to;
		to = tmp;
	}
	const uint32_t span = (uint32_t) to - (uint32_t) from + 1u;
	return (uint16_t) ((uint32_t) from + cxpect_bounded_u32(ctx, span));
}

int32_t
cxpect_rnd_range_i32(cxpect_ctx_t* ctx, int32_t from, int32_t to)
{
	if (from > to)
	{
		const int32_t tmp = from;
		from = to;
		to = tmp;
	}
	const uint32_t first = ((uint32_t) from) ^ 0x80000000u;
	const uint32_t last = ((uint32_t) to) ^ 0x80000000u;
	const uint32_t span = last - first + 1u;
	return cxpect_i32_from_biased(first + cxpect_bounded_u32(ctx, span));
}

uint32_t
cxpect_rnd_range_u32(cxpect_ctx_t* ctx, uint32_t from, uint32_t to)
{
	if (from > to)
	{
		const uint32_t tmp = from;
		from = to;
		to = tmp;
	}
	const uint32_t span = to - from + 1u;
	return from + cxpect_bounded_u32(ctx, span);
}

int64_t
cxpect_rnd_range_i64(cxpect_ctx_t* ctx, int64_t from, int64_t to)
{
	if (from > to)
	{
		const int64_t tmp = from;
		from = to;
		to = tmp;
	}
	const uint64_t first = ((uint64_t) from) ^ UINT64_C(0x8000000000000000);
	const uint64_t last = ((uint64_t) to) ^ UINT64_C(0x8000000000000000);
	const uint64_t span = last - first + 1u;
	return cxpect_i64_from_biased(first + cxpect_bounded_u64(ctx, span));
}

uint64_t
cxpect_rnd_range_u64(cxpect_ctx_t* ctx, uint64_t from, uint64_t to)
{
	if (from > to)
	{
		const uint64_t tmp = from;
		from = to;
		to = tmp;
	}
	const uint64_t span = to - from + 1u;
	return from + cxpect_bounded_u64(ctx, span);
}

float
cxpect_rnd_range_float(cxpect_ctx_t* ctx, float from, float to)
{
	if (from > to)
	{
		const float tmp = from;
		from = to;
		to = tmp;
	}
	const float unit = cxpect_rnd_next_float(ctx);
	if ((from < 0.0f) == (to < 0.0f))
	{
		return from + (to - from) * unit;
	}
	return (1.0f - unit) * from + unit * to;
}

double
cxpect_rnd_range_double(cxpect_ctx_t* ctx, double from, double to)
{
	if (from > to)
	{
		const double tmp = from;
		from = to;
		to = tmp;
	}
	const double unit = cxpect_rnd_next_double(ctx);
	if ((from < 0.0) == (to < 0.0))
	{
		return from + (to - from) * unit;
	}
	return (1.0 - unit) * from + unit * to;
}
	#endif
#endif

#ifndef CXPECT_NO_SHORT_NAMES
	#define before_each(fn) cxpect_before_each(fn)
	#define after_each(fn)	cxpect_after_each(fn)
	#define clear_hooks()	cxpect_clear_hooks()
	#define describe(...)	cxpect_describe(__VA_ARGS__)
	#define it(...)			cxpect_it(__VA_ARGS__)

	#define expect_eq(...)	  cxpect_expect_eq(__VA_ARGS__)
	#define expect_neq(...)	  cxpect_expect_neq(__VA_ARGS__)
	#define expect_gt(...)	  cxpect_expect_gt(__VA_ARGS__)
	#define expect_lt(...)	  cxpect_expect_lt(__VA_ARGS__)
	#define expect_ge(...)	  cxpect_expect_ge(__VA_ARGS__)
	#define expect_le(...)	  cxpect_expect_le(__VA_ARGS__)
	#define expect_true(...)  cxpect_expect_true(__VA_ARGS__)
	#define expect_false(...) cxpect_expect_false(__VA_ARGS__)

	#define expect_match(subject)		   cxpect_expect_match(subject)
	#define expect_match_as(subject, name) cxpect_expect_match_as(subject, name)
	#define when(condition)				   if (cxpect_match_take(&cxpect_internal_match_state.done, !!(condition)))
	#define when_let(condition, field, variable)                                                                       \
		cxpect_when_let(condition, cxpect_internal_match_subject, field, variable)
	#define otherwise()		if (cxpect_match_take(&cxpect_internal_match_state.done, true))
	#define fail_match(msg) cxpect_fail_match(msg)
#endif

#endif // CXPECT_H
