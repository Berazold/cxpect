#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

static unsigned after_calls;

static void
failing_after(cxpect_ctx_t* ctx)
{
	++after_calls;
	expect_eq(after_calls, 0u, "intentional after_each failure");
}

CXPECT_TEST(expected_after_failure)
{
	after_each(failing_after);
	describe("after_each failure", it("is caught exactly once", expect_true(true);););
	clear_hooks();
}

CXPECT_MAIN(cxpect_case(expected_after_failure))
