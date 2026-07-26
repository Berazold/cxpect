#include <stdbool.h>
#include <stdio.h>

typedef struct point
{
	int x;
	int y;
} point;

static bool
point_equals(point left, point right);

static bool
point_greater(point left, point right);

static void
point_print(FILE* file, point value);

#define CXPECT_CUSTOM_EQ                                                                                               \
	point:                                                                                                             \
	point_equals,

#define CXPECT_CUSTOM_NEQ                                                                                              \
	point:                                                                                                             \
	point_equals_not,

#define CXPECT_CUSTOM_GT                                                                                               \
	point:                                                                                                             \
	point_greater,

#define CXPECT_CUSTOM_FORMATTERS                                                                                       \
	point:                                                                                                             \
	point_print,

#define CXPECT_CUSTOM_CONTEXT_MEMBERS unsigned custom_hook_calls;

#define CXPECT_IMPLEMENTATION

#include "../cxpect.h"

static bool
point_equals(point left, point right)
{
	return left.x == right.x && left.y == right.y;
}

static bool
point_equals_not(point left, point right)
{
	return !point_equals(left, right);
}

static bool
point_greater(point left, point right)
{
	return (left.x + left.y) > (right.x + right.y);
}

static void
point_print(FILE* file, point value)
{
	(void) fprintf(file, "point(%d,%d)", value.x, value.y);
}

static void
custom_hook(cxpect_ctx_t* ctx)
{
	++ctx->custom_hook_calls;
}

CXPECT_TEST(customization)
{
	before_each(custom_hook);

	describe("customization",
		it("dispatches custom equality and ordering comparators", const point first = {.x = 2, .y = 3};
			const point same = {.x = 2, .y = 3};
			const point smaller = {.x = 1, .y = 1};

			expect_eq(first, same);
			expect_neq(first, smaller);
			expect_gt(first, smaller);
			expect_eq(ctx->custom_hook_calls, 1u););

		it(
			"dispatches a custom formatter", const point value = {.x = 4, .y = 8}; char buffer[64] = {0};
			FILE* file = tmpfile();
			int flush_result = EOF;
			int seek_result = -1;
			char* read_result = NULL;
			int close_result = EOF;

			if (file != NULL) {
				cxpect_print_generic_val(file, value);
				flush_result = fflush(file);
				seek_result = fseek(file, 0L, SEEK_SET);
				read_result = fgets(buffer, (int) sizeof(buffer), file);
				close_result = fclose(file);
			}

			expect_neq(file, NULL);
			expect_eq(flush_result, 0);
			expect_eq(seek_result, 0);
			expect_neq(read_result, NULL);
			expect_eq(buffer, "point(4,8)");
			expect_eq(close_result, 0);
			expect_eq(ctx->custom_hook_calls, 2u);););

	clear_hooks();
}

CXPECT_MAIN(cxpect_case(customization))
