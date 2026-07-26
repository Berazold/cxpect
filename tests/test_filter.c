#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

CXPECT_TEST(alpha_group)
{
	describe("alpha", it("runs alpha", expect_true(true);););
}

CXPECT_TEST(beta_group)
{
	describe("beta", it("runs beta", expect_true(true);););
}

CXPECT_MAIN(cxpect_case(alpha_group), cxpect_case(beta_group))
