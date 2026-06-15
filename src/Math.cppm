export module CommonLib:Math;

import :Types;
import :TypeTraits;

export {
	namespace CL::Math {

	template<typename T>
	requires IsIntegralV<T> constexpr auto ceil(T x) noexcept -> T
	{
		return x;
	}

	template<typename F>
	requires IsFloatingPointV<F> constexpr auto trunc(F x) noexcept -> F
	{
		auto i = static_cast<long long>(x);
		return static_cast<F>(i);
	}

	template<typename T>
	requires IsIntegralV<T> constexpr T div_ceil(T x, T y) noexcept
	{
		if constexpr (IsUnsignedIntegralV<T>) {
			return x / y + (x % y != 0);
		} else {
			bool q_pos = (x >= 0) == (y >= 0);
			return x / y + (x % y != 0 && q_pos);
		}
	}

	template<typename F>
	requires IsFloatingPointV<F> constexpr F ceil(F x) noexcept
	{
		F t = trunc(x);
		return t + F(t < x);
	}

	template<typename T> constexpr auto median3(T a, T b, T c) -> T
	{
		if (a > b) {
			auto t { a };
			a = b;
			b = t;
		}
		if (b > c) {
			auto t { b };
			b = c;
			c = t;
		}
		if (a > b) {
			auto t { a };
			a = b;
			b = t;
		}
		return b;
	}

	}
}
