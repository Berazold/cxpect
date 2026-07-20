# cxpect

![C11](https://img.shields.io/badge/C-C11-00599C?logo=c&logoColor=white)
![Single header](https://img.shields.io/badge/distribution-single--header-6f42c1)
![Dependencies](https://img.shields.io/badge/dependencies-none-2ea44f)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**cxpect** is a lightweight single-header BDD testing library for C11 and later. It provides expressive `describe`/`it`-style tests, type-aware assertions, hooks, pattern-style matching, optional ULP comparison, and deterministic random testing without external dependencies.

```c
#define CXPECT_IMPLEMENTATION
#include "cxpect.h"

CXPECT_TEST(calculator_spec)
{
    describe("calculator",
        it("adds integers",
            expect_eq(2 + 2, 4);
        );

        it("compares strings",
            expect_eq("cxpect", "cxpect");
            expect_neq("cxpect", "another library");
        );
    );
}

CXPECT_MAIN(cxpect_case(calculator_spec))
```

```console
$ cc -std=c11 -Wall -Wextra -Wpedantic test.c -o test
$ ./test

▶ Running group: calculator_spec

[DESCRIBE] calculator
    [IT] adds integers                                 ✓ OK
    [IT] compares strings                              ✓ OK

✓ All 2 tests completed successfully
```

## Features

- **Single-header distribution** - copy `cxpect.h` into your project and define the implementation once.
- **BDD-style test structure** - organize tests with `describe` and `it`.
- **Type-aware assertions** - integers, floats, booleans, C strings, and pointers.
- **Useful failure diagnostics** - source location, original expression, evaluated values, and optional messages.
- **Test hooks** - `before_each`, `after_each`, and `clear_hooks`.
- **Pattern-style matching** - first-match semantics with `when`, `when_let`, `otherwise`, and fallthrough detection.
- **Optional deterministic random testing** - PCG32-based generators with reproducible seeds and unbiased integer ranges.
- **Optional ULP comparison** - IEEE-754-aware equality for `float` and `double`.
- **Extensible dispatch** - custom comparison functions, value formatters, and context members.
- **Filtering and CI-friendly exit codes** - run matching test groups from the command line and receive a non-zero status on failure.

## Requirements

cxpect targets **C11 and later**.

- GCC and Clang are supported in C11 mode through `__typeof__`.
- C23 compilers use the standard `typeof` operator.
- Other C11 compilers may provide a compatible type operator through the `cxpect_typeof` macro.
- The optional ULP comparator requires IEEE-754 binary32 `float` and binary64 `double` representations.

The library depends only on the C standard library. The optional entropy path uses `/dev/urandom` on unix systems and falls back to an internally generated seed when system entropy is unavailable.

## Installation

Copy [`cxpect.h`](cxpect.h) into your source tree.

Define `CXPECT_IMPLEMENTATION` in **exactly one translation unit** before including the header:

```c
#define CXPECT_IMPLEMENTATION
#include "cxpect.h"
```

Include it normally everywhere else:

```c
#include "cxpect.h"
```

For a small test executable, the implementation and tests can live in the same `.c` file, as shown in the quick-start example.

## Defining and running tests

A test group is declared with `CXPECT_TEST` and registered with `cxpect_case`:

```c
CXPECT_TEST(math_spec)
{
    describe("math operations",
        it("adds integers",
            expect_eq(2 + 3, 5);
        );
    );
}

CXPECT_MAIN(
    cxpect_case(math_spec)
)
```

`CXPECT_MAIN` creates the test context, runs the registered cases, and returns `EXIT_FAILURE` when at least one test fails.

For applications that already provide `main`, use `CXPECT_RUN` or `cxpect_run_tests` directly:

```c
int
main(int argc, char** argv)
{
    cxpect_ctx_t ctx = {
        .argc = (int64_t)argc,
        .argv = argv,
        .filter = argc > 1 ? argv[1] : NULL,
    };

    const cxpect_case_t* cases = CXPECT_CASES(
        cxpect_case(vector_spec)
    );
    cxpect_ctx_post_init(&ctx);
    cxpect_run_tests(&ctx, cases);

    return ctx.tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

## Assertions

Every binary assertion accepts an optional third argument containing a failure message.

| Assertion | Meaning |
|---|---|
| `expect_eq(actual, expected[, message])` | Equal |
| `expect_neq(actual, expected[, message])` | Not equal |
| `expect_gt(actual, expected[, message])` | Greater than |
| `expect_lt(actual, expected[, message])` | Less than |
| `expect_ge(actual, expected[, message])` | Greater than or equal |
| `expect_le(actual, expected[, message])` | Less than or equal |
| `expect_true(expression[, message])` | Expression evaluates to true |
| `expect_false(expression[, message])` | Expression evaluates to false |

```c
it("supports typed assertions",
    int value = 42;
    int* pointer = &value;

    expect_eq(value, 42);
    expect_gt(value, 0, "value must be positive");
    expect_eq("hello", "hello");
    expect_neq(pointer, NULL);
    expect_true(pointer != NULL);
);
```

Assertion operands and messages are evaluated once.

### Supported built-in types

Equality and ordering are dispatched with C11 `_Generic`.

- All standard signed and unsigned integer types
- `bool`
- `float`, `double`, and `long double`
- Mutable and immutable C strings
- Object pointers for equality and inequality

C strings are compared by content. String ordering is lexicographical. Pointer ordering is intentionally not provided.

Mixed arithmetic operands follow the usual C arithmetic conversions before dispatch.

## Floating-point equality

By default, floating-point equality uses exact equality plus an absolute epsilon comparison. The thresholds can be configured before including `cxpect.h`:

```c
#define CXPECT_FLOAT_ABS_EPSILON       1e-5f
#define CXPECT_DOUBLE_ABS_EPSILON      1e-9
#define CXPECT_LONG_DOUBLE_ABS_EPSILON 1e-12L
#include "cxpect.h"
```

Enable ULP-aware comparison for `float` and `double` with:

```c
#define CXPECT_ENABLE_ULP_COMPARE
#include "cxpect.h"
```

ULP mode accepts exact matches, nearby values covered by its epsilon fallback, and values separated by at most one ULP. Its special-value policies are configurable:

```c
#define CXPECT_ENABLE_ULP_COMPARE
#define CXPECT_MATCH_NAN_EQUALS_NAN 0
#include "cxpect.h"
```

`long double` currently remains absolute-epsilon based because its representation varies across platforms.

## Hooks

Hooks receive the active `cxpect_ctx_t*` and run around every `it` block:

```c
static void
setup(cxpect_ctx_t* ctx)
{
    /* prepare test state */
}

static void
teardown(cxpect_ctx_t* ctx)
{
    /* release/cleanup test state */
}

CXPECT_TEST(hook_spec)
{
    before_each(setup);
    after_each(teardown);

    describe("hooks",
        it("runs with setup and teardown",
            expect_true(true);
        );
    );

    clear_hooks();
}
```

`after_each` still runs when an assertion fails inside the test body. A failure inside `after_each` marks the test as failed.

## Pattern-style matching

The match API evaluates the subject once and executes only the first matching branch:

```c
typedef struct payload
{
    int kind;
    int value;
} payload;

it("matches structured values",
    const payload item =
    {
        .kind = 1,
        .value = 42,
    };
    int captured = 0;

    expect_match(item)
    {
        when(item.kind == 0)
        {
            fail_match("unexpected kind");
        }

        when_let(item.kind == 1, value, bound_value)
        {
            captured = bound_value;
        }

        otherwise()
        {
            fail_match("unhandled kind");
        }
    }

    expect_eq(captured, 42);
);
```

Use `expect_match_as` when the subject should be bound to a chosen name:

```c
expect_match_as(compute_value(), value)
{
    when(value < 0)
    {
        fail_match("negative value");
    }

    otherwise()
    {
        expect_ge(value, 0);
    }
}
```

A match with no selected branch fails automatically, making missing cases visible instead of silently doing nothing.

## Random testing

Enable the random API before including the implementation:

```c
#define CXPECT_ENABLE_RANDOM
#define CXPECT_IMPLEMENTATION
#include "cxpect.h"
```

```c
it("generates bounded values",
    for (int index = 0; index < 1000; ++index)
    {
        const int value = cxpect_rnd_range_i32(ctx, -10, 10);
        expect_ge(value, -10);
        expect_le(value, 10);
    }
);
```

Available generators include signed and unsigned 8, 16, 32 and 64-bit integers, `float`, and `double`. Integer range functions use inclusive bounds and accept endpoints in either order.

When a random-enabled suite fails, cxpect prints a reproducible seed:

```console
Reproduce with: CXPECT_SEED=0123456789abcdef:fedcba9876543210
```

Replay it through the environment:

```console
$ CXPECT_SEED=0123456789abcdef:fedcba9876543210 ./test
```

Or define a compile-time seed:

```c
#define CXPECT_DETERMINISTIC_SEED "0123456789abcdef:fedcba9876543210"
```

## Filtering test groups

The first command-line argument is treated as a substring filter for registered case names:

```console
$ ./test vector
```

Only cases whose registered names contain `vector` will run.

## Custom types

Custom comparison and formatting entries are added to the library's `_Generic` dispatch before including the header:

```c
#include <stdbool.h>
#include <stdio.h>

typedef struct point
{
    int x;
    int y;
} point;

static bool point_equals(point left, point right);
static void point_print(FILE* file, point value);

#define CXPECT_CUSTOM_EQ         point: point_equals,
#define CXPECT_CUSTOM_FORMATTERS point: point_print,
#define CXPECT_IMPLEMENTATION
#include "cxpect.h"

static bool
point_equals(point left, point right)
{
    return left.x == right.x && left.y == right.y;
}

static void
point_print(FILE* file, point value)
{
    (void)fprintf(file, "point(%d, %d)", value.x, value.y);
}
```

The available customization macros are:

- `CXPECT_CUSTOM_EQ`
- `CXPECT_CUSTOM_NEQ`
- `CXPECT_CUSTOM_GT`
- `CXPECT_CUSTOM_LT`
- `CXPECT_CUSTOM_GE`
- `CXPECT_CUSTOM_LE`
- `CXPECT_CUSTOM_FORMATTERS`
- `CXPECT_CUSTOM_CONTEXT_MEMBERS`

## Configuration

Define configuration macros before including `cxpect.h`.

| Macro | Effect |
|---|---|
| `CXPECT_IMPLEMENTATION` | Emits the implementation. define in exactly one translation unit |
| `CXPECT_NO_SHORT_NAMES` | Disables aliases such as `describe`, `it`, and `expect_eq` |
| `CXPECT_NO_COLORED_OUTPUT` | Disables ANSI terminal colors |
| `CXPECT_ENABLE_RANDOM` | Enables PCG32-based random generators |
| `CXPECT_DETERMINISTIC_SEED` | Sets a compile-time reproducible random seed |
| `CXPECT_ENABLE_ULP_COMPARE` | Enables ULP-aware `float` and `double` equality |
| `CXPECT_MATCH_NAN_EQUALS_NAN` | Controls NaN equality in ULP mode; default `0` |
| `CXPECT_MATCH_INF_EQUALS_INF` | Controls same-sign infinity equality in ULP mode. default `1` |
| `CXPECT_FLOAT_ABS_EPSILON` | Configures absolute epsilon for `float` |
| `CXPECT_DOUBLE_ABS_EPSILON` | Configures absolute epsilon for `double` |
| `CXPECT_LONG_DOUBLE_ABS_EPSILON` | Configures absolute epsilon for `long double` |
| `cxpect_fail_impl` | Overrides the fatal internal failure primitive |
| `cxpect_typeof` | Overrides the type-inference operator |

When `CXPECT_NO_SHORT_NAMES` is enabled, use the prefixed forms such as `cxpect_describe`, `cxpect_it`, and `cxpect_expect_eq`.

## Failure and cleanup model

Assertion failures use `setjmp`/`longjmp` to leave the current `it` block and continue the suite. Consequently, ordinary control flow and automatic cleanup code after a failed assertion do not run.

Place essential cleanup in `after_each`, or release resources before an assertion that may fail. Do not rely on returning through intermediate stack frames after a failed assertion.

## Project status and portability

cxpect is intentionally small and has no build-system dependency. The repository targets GCC and Clang. Ports to other compilers should provide `cxpect_typeof` when neither C23 `typeof` nor `__typeof__` is available.

The ULP implementation is intentionally limited to platforms matching the compile-time IEEE-754 binary32 and binary64 checks. Builds that do not satisfy those requirements fail at compile time when ULP mode is enabled.

## License

cxpect is released under the [MIT License](LICENSE).

The optional random-testing module contains a modified PCG32 implementation used under the MIT License. The original third-party copyright and permission notice is preserved directly in [`cxpect.h`](cxpect.h).
