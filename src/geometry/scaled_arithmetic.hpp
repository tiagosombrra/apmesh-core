#pragma once

#include <cmath>
#include <cstdint>
#include <limits>

namespace apmesh::core::detail {

// Private spline intermediate: significand * 2^exponent. This extends range,
// not precision. Never materialize the scale until the public result boundary.
class Scaled {
public:
    Scaled() noexcept = default;
    Scaled(const long double value) noexcept {
        if (std::isfinite(value)) {
            significand_ = std::frexp(value, &exponent_);
        } else {
            significand_ = std::numeric_limits<long double>::quiet_NaN();
        }
    }

    [[nodiscard]] bool finite() const noexcept {
        return std::isfinite(significand_);
    }

    [[nodiscard]] explicit operator double() const noexcept {
        return static_cast<double>(std::scalbn(significand_, exponent_));
    }

    [[nodiscard]] friend Scaled operator-(const Scaled value) noexcept {
        Scaled result = value;
        result.significand_ = -result.significand_;
        return result;
    }

    [[nodiscard]] friend Scaled operator+(
        Scaled lhs, Scaled rhs) noexcept {
        if (!lhs.finite() || !rhs.finite()) {
            return invalid();
        }
        if (lhs.significand_ == 0.0L) {
            return rhs;
        }
        if (rhs.significand_ == 0.0L) {
            return lhs;
        }
        if (lhs.exponent_ < rhs.exponent_) {
            const Scaled swap = lhs;
            lhs = rhs;
            rhs = swap;
        }
        const std::int64_t gap =
            static_cast<std::int64_t>(lhs.exponent_) - rhs.exponent_;
        // Below a quarter ULP of the larger significand, the smaller operand
        // cannot change a round-to-nearest sum. Avoid underflow in alignment.
        if (gap > std::numeric_limits<long double>::digits + 2) {
            return lhs;
        }
        return normalized(
            lhs.significand_ +
                std::scalbn(rhs.significand_, -static_cast<int>(gap)),
            lhs.exponent_);
    }

    [[nodiscard]] friend Scaled operator-(
        const Scaled lhs, const Scaled rhs) noexcept {
        return lhs + (-rhs);
    }

    [[nodiscard]] friend Scaled operator*(
        const Scaled lhs, const Scaled rhs) noexcept {
        if (!lhs.finite() || !rhs.finite()) {
            return invalid();
        }
        return normalized(
            lhs.significand_ * rhs.significand_,
            static_cast<std::int64_t>(lhs.exponent_) + rhs.exponent_);
    }

    [[nodiscard]] friend Scaled operator/(
        const Scaled lhs, const Scaled rhs) noexcept {
        if (!lhs.finite() || !rhs.finite() || rhs.significand_ == 0.0L) {
            return invalid();
        }
        return normalized(
            lhs.significand_ / rhs.significand_,
            static_cast<std::int64_t>(lhs.exponent_) - rhs.exponent_);
    }

    [[nodiscard]] friend bool operator==(
        const Scaled lhs, const Scaled rhs) noexcept {
        return lhs.significand_ == rhs.significand_ &&
               (lhs.significand_ == 0.0L || lhs.exponent_ == rhs.exponent_);
    }

    [[nodiscard]] friend bool operator<(
        const Scaled lhs, const Scaled rhs) noexcept {
        if (!lhs.finite() || !rhs.finite()) {
            return false;
        }
        if (lhs.significand_ == 0.0L || rhs.significand_ == 0.0L ||
            (lhs.significand_ < 0.0L) != (rhs.significand_ < 0.0L)) {
            return lhs.significand_ < rhs.significand_;
        }
        if (lhs.exponent_ == rhs.exponent_) {
            return lhs.significand_ < rhs.significand_;
        }
        return lhs.significand_ > 0.0L
                   ? lhs.exponent_ < rhs.exponent_
                   : lhs.exponent_ > rhs.exponent_;
    }

    [[nodiscard]] friend bool operator>(
        const Scaled lhs, const Scaled rhs) noexcept { return rhs < lhs; }
    [[nodiscard]] friend bool operator<=(
        const Scaled lhs, const Scaled rhs) noexcept {
        return lhs < rhs || lhs == rhs;
    }

private:
    [[nodiscard]] static Scaled invalid() noexcept {
        return Scaled{std::numeric_limits<long double>::quiet_NaN()};
    }

    [[nodiscard]] static Scaled normalized(
        const long double significand, const std::int64_t exponent) noexcept {
        if (!std::isfinite(significand)) {
            return invalid();
        }
        if (significand == 0.0L) {
            return Scaled{significand};
        }
        int shift{};
        const long double fraction = std::frexp(significand, &shift);
        const std::int64_t combined = exponent + shift;
        if (combined > std::numeric_limits<int>::max() ||
            combined < std::numeric_limits<int>::min()) {
            return invalid();
        }
        Scaled result;
        result.significand_ = fraction;
        result.exponent_ = static_cast<int>(combined);
        return result;
    }

    long double significand_{};
    int exponent_{};
};

[[nodiscard]] inline bool is_finite(const Scaled value) noexcept {
    return value.finite();
}

[[nodiscard]] inline bool is_finite(const double value) noexcept {
    return std::isfinite(value);
}

[[nodiscard]] inline Scaled lerp(
    const Scaled lhs, const Scaled rhs, const Scaled parameter) noexcept {
    if (parameter == 0.0L) {
        return lhs;
    }
    if (parameter == 1.0L || lhs == rhs) {
        return rhs;
    }
    return (Scaled{1.0L} - parameter) * lhs + parameter * rhs;
}

} // namespace apmesh::core::detail
