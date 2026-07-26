#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

CXPECT_TEST(array_case_one)
{
	describe("case arrays", it("runs the first case", expect_eq(1, 1);););
}

CXPECT_TEST(array_case_two)
{
	describe("case arrays", it("runs the second case", expect_eq(2, 2);););
}

int
main(int argc, char** argv)
{
	cxpect_ctx_t ctx = {
		.argv = argv,
		.filter = argc > 1 ? argv[1] : NULL,
		.argc = (int64_t) argc,
	};
	const cxpect_case_t* cases = CXPECT_CASES(cxpect_case(array_case_one), cxpect_case(array_case_two));

	cxpect_ctx_post_init(&ctx);
	cxpect_run_tests(&ctx, cases);
	return ctx.tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
