#define CXPECT_ENABLE_ULP_COMPARE
#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

static float
float_from_bits(uint32_t bits)
{
	cxpect_math_mask_32 value = {.u = bits};
	return value.f;
}

static double
double_from_bits(uint64_t bits)
{
	cxpect_math_mask_64 value = {.u = bits};
	return value.d;
}

CXPECT_TEST(ulp_comparison)
{
	describe("ULP comparison",
		it("handles exact values, signed zero, epsilon, and adjacent ULPs",
			const float float_large = float_from_bits(0x4b000000u);
			const float float_next = float_from_bits(0x4b000001u);
			const float float_two_away = float_from_bits(0x4b000002u);
			const double double_large = double_from_bits(0x4330000000000000u);
			const double double_next = double_from_bits(0x4330000000000001ul);
			const double double_two_away = double_from_bits(0x4330000000000002ul);

			expect_true(cxpect_math_float_equals(1.0f, 1.0f));
			expect_true(cxpect_math_float_equals(0.0f, -0.0f));
			expect_true(cxpect_math_float_equals(5e-8f, -5e-8f));
			expect_true(cxpect_math_float_equals(float_large, float_next));
			expect_false(cxpect_math_float_equals(float_large, float_two_away));

			expect_true(cxpect_math_double_equals(1.0, 1.0));
			expect_true(cxpect_math_double_equals(0.0, -0.0));
			expect_true(cxpect_math_double_equals(5e-13, -5e-13));
			expect_true(cxpect_math_double_equals(double_large, double_next));
			expect_false(cxpect_math_double_equals(double_large, double_two_away));

			expect_true(cxpect_math_long_double_equals(1.0L, 1.0L + 0.5e-5L));
			expect_false(cxpect_math_long_double_equals(1.0L, 1.0L + 2.0e-5L)););

		it("handles NaN and infinity according to the default policy",
			const float float_nan = float_from_bits(0x7fc00001u);
			const float float_inf = float_from_bits(0x7f800000u);
			const double double_nan = double_from_bits(0x7ff8000000000001ul);
			const double double_inf = double_from_bits(0x7ff0000000000000ul);

			expect_false(cxpect_math_float_equals(float_nan, float_nan));
			expect_true(cxpect_math_float_equals(float_inf, float_inf));
			expect_false(cxpect_math_float_equals(float_inf, -float_inf));

			expect_false(cxpect_math_double_equals(double_nan, double_nan));
			expect_true(cxpect_math_double_equals(double_inf, double_inf));
			expect_false(cxpect_math_double_equals(double_inf, -double_inf));););
}

CXPECT_MAIN(cxpect_case(ulp_comparison))
