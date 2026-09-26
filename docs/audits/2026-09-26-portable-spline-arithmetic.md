# Portable spline intermediate arithmetic

Date: 2026-09-26
Status: SIX DEVELOPMENT CELLS PASS / VALIDATED_UNMERGED / NOT QUALIFIED
Scope: bounded portability maintenance, not scientific qualification

## Baseline and failure

The implementation baseline is protected-main revision
`23c9a93d6b06f10a33b4ce41b8fc18f801b7752e`. The native Windows preset
lineage is `build/native-windows-msvc-presets`, with preset/documentation
checkpoint `a9355cd597e56e33808f8a9497d70a553006d8fd`.

The baseline ordinary suite passes 40/40 under GCC Debug. MSVC Debug builds
successfully but passes 36/40: the extreme-finite contracts fail in
`two_span_cubic_bspline`, `two_span_cubic_nurbs`, `multi_span_cubic_nurbs`
and `surface_bicubic_nurbs`. Knot differences and derivative controls can
overflow, and positive normalized weights can underflow, before a finite
public result is recovered. Wider `long double` on Linux masked these range
failures; MSVC gives `long double` the same representation as `double`.

## Correction and preserved boundaries

The private helper `src/geometry/scaled_arithmetic.hpp` represents each
intermediate as `significand * 2^exponent`. `frexp` normalizes the native
`long double` significand; a separately checked integer exponent carries the
range. Products and quotients operate on bounded significands. Sums align
their exponents; terms below a quarter ULP of the larger significand cannot
change its round-to-nearest result and need not be materialized. Integer
exponent overflow returns an invalid intermediate rather than overflowing.

All four paths use this representation for knot ratios, control differences,
local homogeneous controls, de Boor interpolation and rational jets through
second order, including the mixed surface partial. The earlier
overflow-triggered ratio fallback is removed: there is one scaled path, not
a retry or rescue. Existing local-support and positive common-weight-scale
rules are retained.

The conversion to binary64 occurs only at the public result boundary, with
the existing finite/range checks. Nonrepresentable large results still return
`CurveError::non_finite_result` or `SurfaceError::non_finite_result`.
Final subnormal rounding/underflow retains the native round-to-nearest
conversion policy. No public headers, stored controls/weights/knots,
continuity rules, error enums, tolerances or acceptance gates change.
There are no third-party dependencies or legacy-code imports.

This extends intermediate **range, not precision**. Native significand
precision is still compiler-dependent; arbitrary cancellation accuracy,
exact arithmetic, interval certification and cross-compiler bitwise identity
are not claimed. No unsafe floating-point compilation flags are introduced.

## Focused evidence

The new ordinary `apmesh_core.scaled_spline_arithmetic` contract covers:

- overflow-range differences and derivative scales recovered into binary64;
- positive weight ratios below binary64's subnormal range;
- cancellation, signed ordering, interpolation endpoints and constants;
- every binary64 power-of-two exponent from -1074 through 1023;
- explicit invalid propagation and final subnormal tie-to-even rounding;
- affine two-span B-splines, two-span NURBS, multi-span NURBS and bicubic
  NURBS surfaces with extreme finite knots/coordinates and uniform weights
  at both binary64 extremes. Analytic values and first/second jets are
  asserted, not just finiteness.

The original 40 contracts remain enabled and unchanged. The ordinary FAST
and INTEGRATION inventory becomes 41; qualification tooling stays disabled.

| Environment | Debug FAST | Release FAST |
| --- | --- | --- |
| WSL Ubuntu 24.04, GCC 13.3 / libstdc++ | PASS, 41/41 | PASS, 41/41 |
| WSL Ubuntu 24.04, Clang 18.1.3 / libc++ | PASS, 41/41 | PASS, 41/41 |
| Native Windows, MSVC 19.44.35229 x64 | PASS, 41/41 | PASS, 41/41 |

The strengthened spline-only Debug checks passed on 2026-09-26,
18:58:37--18:58:59 Fortaleza time. Initial Release checks started at 18:59:16;
Clang/MSVC passed and GCC retained the build failure below. After the
separately authorized sorting correction, final Release checks ran from
19:02:10 through 19:03:02 and final Debug checks from 19:02:28 through
19:03:41. All six final cells configured, built and passed 41/41.

External local logs and JSON summaries are retained under temporary-directory
basenames `apmesh-nurbs-portability-final-20260926` (initial evidence) and
`apmesh-nurbs-portability-sort-final-20260926` (final evidence); they are not
versioned artifacts or formal campaign evidence.

The initial GCC Release configure passed, but `-O3 -Werror=array-bounds`
stopped the build in the then-unchanged `inflection_evidence` function at
`curve.cpp:1308`.
The diagnostic originates in the inlined `std::sort` implementation for a
two-element bracket array. This is recorded separately from spline arithmetic;
no warning suppression or optimization relaxation was applied.

The separately authorized mechanical correction replaces that dynamic
`std::sort` range with one compare/swap when the fixed capacity of two is
occupied. The lexicographic comparator (lower parameter, then upper parameter)
is identical; zero/one-bracket cases remain untouched. The existing quadratic
capacity guard, interval enclosures, sign proofs, resource limits and failure
classifications are unchanged. The existing inflection-isolation contract
preserves zero/one/two-root, ordered/disjoint-bracket, reversal, determinism
and indeterminate evidence. All six cells passed after this additional source
change; the initial failed build/log remains retained. No build blocker remains
in this six-cell development matrix.

Reproduction uses the existing development presets:

```sh
cmake --preset gcc-debug
cmake --build --preset gcc-debug --parallel 2
ctest --preset gcc-debug -L fast
```

Substitute `gcc-release`, `clang-debug` or `clang-release` for the Linux
cell. On native Windows use `windows-msvc-debug` or
`windows-msvc-release` for all three commands. These presets leave formal
qualification tooling off. All 41 ordinary contracts also carry the
INTEGRATION label; no selected ordinary contract is omitted.

## Authority and sources

This correction implements the existing extreme-finite/no-avoidable-overflow
obligations in `CURVE_TWO_SPAN_CUBIC_BSPLINE_DECISION.md`,
`CURVE_TWO_SPAN_CUBIC_NURBS_DECISION.md`,
`CURVE_MULTI_SPAN_CUBIC_NURBS_DECISION.md` and
`SURFACE_BICUBIC_NURBS_DECISION.md`. The Numeric Contract's binary64,
round-to-nearest and explicit-failure policies remain unchanged.

- [Microsoft: floating-point representation](https://learn.microsoft.com/en-us/cpp/build/ieee-floating-point-representation?view=msvc-170)
  documents the MSVC `long double` representation limitation.
- [GNU C Library: normalization functions](https://ftp.gnu.org/old-gnu/Manuals/glibc/html_node/Normalization-Functions.html)
  documents significand/exponent decomposition and binary scaling.

No stage is advanced or requalified. Native Windows remains development-only.
The next scientific breadth comparison remains deferred until this maintenance
item is integrated and its post-merge checks and continuity records are closed.
