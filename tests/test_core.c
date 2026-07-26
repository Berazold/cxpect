#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

static int before_calls;
static int after_calls;

static void
core_before(cxpect_ctx_t* ctx)
{
	(void) ctx;
	++before_calls;
}

static void
core_after(cxpect_ctx_t* ctx)
{
	(void) ctx;
	++after_calls;
}

CXPECT_TEST(core_assertions)
{
	describe("core assertions",
		it("supports scalar, string, pointer, and unary assertions",
			{
				int value = 7;
				int* pointer = &value;
				int* null_pointer = NULL;

				expect_eq((signed char) -2, (signed char) -2);
				expect_neq((unsigned short) 2, (unsigned short) 3, "optional messages compile");
				expect_gt(4, 3);
				expect_lt(3u, 4u);
				expect_ge(4L, 4L);
				expect_le(4ULL, 5ULL);
				expect_eq(1.0f, 1.0f);
				expect_eq(2.0, 2.0);
				expect_eq(3.0L, 3.0L);
				expect_eq("cxpect", "cxpect");
				expect_neq("cxpect", "other");
				expect_eq((const char*) NULL, (const char*) NULL);
				expect_eq(pointer, &value);
				expect_eq(null_pointer, NULL);
				expect_true(pointer != NULL);
				expect_false(false);
			});

		it("evaluates assertion operands and messages once", int actual_evaluations = 0; int expected_evaluations = 0;
			int message_evaluations = 0;
			const char* messages[] = {"evaluated once"};

			expect_eq(++actual_evaluations, ++expected_evaluations, messages[message_evaluations++]);
			expect_eq(actual_evaluations, 1);
			expect_eq(expected_evaluations, 1);
			expect_eq(message_evaluations, 1);););
}

CXPECT_TEST(core_hooks)
{
	before_calls = 0;
	after_calls = 0;
	before_each(core_before);
	after_each(core_after);

	describe("hooks",
		it("runs before_each and after_each around every test",
			{
				expect_eq(before_calls, 1);
				expect_eq(after_calls, 0);
			});

		it("preserves hook state across sibling tests", {
			expect_eq(before_calls, 2);
			expect_eq(after_calls, 1);
		}););

	clear_hooks();

	describe("cleared hooks", it("stops invoking cleared hooks", {
		expect_eq(before_calls, 2);
		expect_eq(after_calls, 2);
	}););
}

CXPECT_MAIN(cxpect_case(core_assertions), cxpect_case(core_hooks))
