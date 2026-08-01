#define CXPECT_ENABLE_RANDOM
#define CXPECT_DETERMINISTIC_SEED "0123456789abcdef:fedcba9876543210"
#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

CXPECT_TEST(random_api)
{
	describe("random API",
		it("replays the same deterministic sequence",
			{
				cxpect_ctx_t first = {0};
				cxpect_ctx_t second = {0};
				cxpect_rnd_init(&first);
				cxpect_rnd_init(&second);

				for (int index = 0; index < 64; ++index)
				{
					expect_eq(cxpect_rnd_next_u32(&first), cxpect_rnd_next_u32(&second));
					expect_eq(cxpect_rnd_next_u64(&first), cxpect_rnd_next_u64(&second));
				}
			});

		it(
			"keeps every bounded generator inside inclusive limits", for (int index = 0; index < 2048; ++index) {
				const int8_t i8 = cxpect_rnd_range_i8(cxpect_default_ctx, INT8_C(-7), INT8_C(5));
				const uint8_t u8 = cxpect_rnd_range_u8(cxpect_default_ctx, UINT8_C(9), UINT8_C(3));
				const int16_t i16 = cxpect_rnd_range_i16(cxpect_default_ctx, INT16_C(-500), INT16_C(700));
				const uint16_t u16 = cxpect_rnd_range_u16(cxpect_default_ctx, UINT16_C(1000), UINT16_C(20));
				const int32_t i32 = cxpect_rnd_range_i32(cxpect_default_ctx, INT32_MIN, INT32_MAX);
				const uint32_t u32 = cxpect_rnd_range_u32(cxpect_default_ctx, UINT32_C(100), UINT32_C(200));
				const int64_t i64 = cxpect_rnd_range_i64(cxpect_default_ctx, INT64_MIN, INT64_MAX);
				const uint64_t u64 = cxpect_rnd_range_u64(cxpect_default_ctx, UINT64_C(900), UINT64_C(100));
				const float f32 = cxpect_rnd_range_float(cxpect_default_ctx, -10.0f, 20.0f);
				const double f64 = cxpect_rnd_range_double(cxpect_default_ctx, 50.0, -25.0);

				expect_ge(i8, INT8_C(-7));
				expect_le(i8, INT8_C(5));
				expect_ge(u8, UINT8_C(3));
				expect_le(u8, UINT8_C(9));
				expect_ge(i16, INT16_C(-500));
				expect_le(i16, INT16_C(700));
				expect_ge(u16, UINT16_C(20));
				expect_le(u16, UINT16_C(1000));
				expect_ge(i32, INT32_MIN);
				expect_le(i32, INT32_MAX);
				expect_ge(u32, UINT32_C(100));
				expect_le(u32, UINT32_C(200));
				expect_ge(i64, INT64_MIN);
				expect_le(i64, INT64_MAX);
				expect_ge(u64, UINT64_C(100));
				expect_le(u64, UINT64_C(900));
				expect_ge(f32, -10.0f);
				expect_le(f32, 20.0f);
				expect_ge(f64, -25.0);
				expect_le(f64, 50.0);
			}););
}

CXPECT_MAIN(cxpect_case(random_api))
