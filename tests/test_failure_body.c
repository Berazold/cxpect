#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

CXPECT_TEST(expected_failure)
{
	describe("failure handling",													 //
		it("records an assertion failure", expect_eq(1, 2, "intentional failure");); //
		it("continues with the next test", expect_true(true););						 //
	);
}

CXPECT_MAIN(cxpect_case(expected_failure))
