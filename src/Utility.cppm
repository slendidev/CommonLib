export module CommonLib:Utility;

import :String;
import :StringView;
import :TypeTraits;
import :Types;
export import :UtilityBase;

export {
	namespace CL {

	template<typename CharTypeT> struct BaseString;
	template<typename CharTypeT> struct BaseStringView;

	namespace detail::adl {

	constexpr auto hash_string(BaseStringView<char> const) -> usize;
	auto to_display_string(BaseStringView<char> const) -> BaseString<char>;
	auto to_display_string(char const *) -> BaseString<char>;

	auto to_debug_string(BaseStringView<char> const) -> BaseString<char>;
	auto to_debug_string(char const *) -> BaseString<char>;

	auto to_hash(BaseStringView<char> const) -> usize;
	auto to_hash(char const *) -> usize;

	template<typename T>
	requires requires(T const &value) { value.view(); }
	constexpr auto to_display_string(T const &value)
	{
		return to_display_string(value.view());
	}

	template<typename T>
	requires requires(T const &value) { value.view(); }
	constexpr auto to_debug_string(T const &value)
	{
		return to_debug_string(value.view());
	}

	template<typename T>
	requires requires(T const &value) { value.view(); }
	constexpr auto to_hash(T const &value) -> usize
	{
		return to_hash(value.view());
	}

	template<typename T>
	requires(IsIntegralV<RemoveConstRef<T>>)
	constexpr auto to_hash(T value) -> usize
	{
		return static_cast<usize>(value);
	}

	}

	inline constexpr struct ToDisplayStringFn {
		template<typename T>
		constexpr auto operator()(T &&value) const -> decltype(auto)
		{
			using detail::adl::to_display_string;
			return to_display_string(forward<T>(value));
		}
	} to_display_string { };

	inline constexpr struct ToDebugStringFn {
		template<typename T>
		constexpr auto operator()(T &&value) const -> decltype(auto)
		{
			using detail::adl::to_debug_string;
			return to_debug_string(forward<T>(value));
		}
	} to_debug_string { };

	inline constexpr struct ToHashFn {
		template<typename T>
		constexpr auto operator()(T &&value) const -> decltype(auto)
		{
			using detail::adl::to_hash;
			return to_hash(forward<T>(value));
		}
	} to_hash { };

	}
}

export constexpr auto CL::detail::adl::hash_string(StringView const value)
    -> usize
{
	usize hash { 14695981039346656037ull };

	for (usize i { }; i < value.size(); ++i) {
		hash ^= static_cast<unsigned char>(value.data()[i]);
		hash *= 1099511628211ull;
	}

	return hash;
}

export inline auto CL::detail::adl::to_display_string(StringView const value)
    -> String
{
	return String(value);
}

export inline auto CL::detail::adl::to_display_string(char const *value)
    -> String
{
	return String(value);
}

export inline auto CL::detail::adl::to_debug_string(StringView const value)
    -> String
{
	return String(value);
}

export inline auto CL::detail::adl::to_debug_string(char const *value) -> String
{
	return String(value);
}

export inline auto CL::detail::adl::to_hash(StringView const value) -> usize
{
	return hash_string(value);
}

export inline auto CL::detail::adl::to_hash(char const *value) -> usize
{
	return hash_string(StringView(value));
}
