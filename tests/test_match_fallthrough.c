#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

CXPECT_TEST(expected_match_failure)
{
    describe("match fallthrough",
        it("reports an unhandled subject",
            expect_match_as(5, value)
            {
                when(value == 1) { expect_true(true); }
            }
        );
    );
}

CXPECT_MAIN(cxpect_case(expected_match_failure))
