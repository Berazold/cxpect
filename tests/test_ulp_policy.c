#define CXPECT_ENABLE_ULP_COMPARE
#define CXPECT_MATCH_NAN_EQUALS_NAN 1
#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

CXPECT_TEST(ulp_policy)
{
	describe("ULP policy switches", it("can match NaNs while rejecting equal infinities", {
		const cxpect_math_mask_32 float_nan = {.u = 0x7fc00001u};
		const cxpect_math_mask_32 float_inf = {.u = 0x7f800000u};
		const cxpect_math_mask_64 double_nan = {.u = 0x7ff8000000000001ul};
		const cxpect_math_mask_64 double_inf = {.u = 0x7ff0000000000000ul};

		expect_eq(float_nan.f, float_nan.f);
		expect_eq(float_inf.f, float_inf.f);
		expect_eq(double_nan.d, double_nan.d);
		expect_eq(double_inf.d, double_inf.d);
	}););
}

CXPECT_MAIN(cxpect_case(ulp_policy))
