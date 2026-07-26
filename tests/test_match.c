#define CXPECT_IMPLEMENTATION
#include "../cxpect.h"

typedef struct match_payload
{
    int kind;
    int value;
} match_payload;

CXPECT_TEST(match_api)
{
    describe("match API",
        it("selects the first matching branch and evaluates the subject once",
            int subject_evaluations = 0;
            int selected = 0;

            expect_match_as(++subject_evaluations, value)
            {
                when(value == 0)
                {
                    selected = -1;
                }
                when(value == 1)
                {
                    selected = 1;
                }
                otherwise()
                {
                    selected = 2;
                }
            }

            expect_eq(subject_evaluations, 1);
            expect_eq(selected, 1);
        );

        it("supports otherwise and field binding with when_let",
            const match_payload payload =
			{
				.kind = 7,
				.value = 42,
			};
            int captured = 0;

            expect_match(payload)
            {
                when_let(true, value, bound_value)
                {
                    captured = bound_value;
                }
                otherwise()
                {
                    fail_match("otherwise must not run");
                }
            }

            expect_eq(captured, 42);

            expect_match_as(99, number)
            {
                when(number < 0)
                {
                    fail_match("negative branch must not run");
                }
                otherwise()
                {
                    expect_eq(number, 99);
                }
            }
        );
    );
}

CXPECT_MAIN(cxpect_case(match_api))
