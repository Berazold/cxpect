#define CXPECT_NO_SHORT_NAMES
#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

CXPECT_TEST(prefixed_api)
{
	cxpect_describe("prefixed API", cxpect_it("works without short names", {
		cxpect_expect_eq(10, 10);
		cxpect_expect_true(true);
	}););
}

CXPECT_MAIN(cxpect_case(prefixed_api))
