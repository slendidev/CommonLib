export module CommonLib:UtilityBase;

export {
	namespace CL {

	template<typename T> T &&move(T &value) { return static_cast<T &&>(value); }

	template<typename T> constexpr T &&forward(T &value) noexcept
	{
		return static_cast<T &&>(value);
	}

	template<typename T> constexpr T &&forward(T &&value) noexcept
	{
		return static_cast<T &&>(value);
	}

	template<typename T> constexpr void swap(T &a, T &b)
	{
		T tmp = CL::move(a);
		a = CL::move(b);
		b = CL::move(tmp);
	}

	template<typename T> struct InPlace { };

	template<typename... Ts> [[gnu::always_inline]] void ignore_unused(Ts &&...)
	{
	}

	template<typename... Ts>
	[[gnu::always_inline]] void ignore_unused(Ts const &...)
	{
	}

	template<typename... Ts> [[gnu::always_inline]] void ignore_unused() { }

	}
}
