export module CommonLib:Errors;

import :Platform;
import :StringView;
import :Variant;

export {
	namespace CL {

	template<typename... Es> using Error = Variant<Es...>;

	namespace ErrorsV {
	struct HashMapDuplicateKeyError { };
	struct InvalidIndex { };
	struct OutOfMemory { };
	struct PoppingEmptyList { };
	}

	using Errors = Error<ErrorsV::HashMapDuplicateKeyError,
	    ErrorsV::InvalidIndex, ErrorsV::OutOfMemory, ErrorsV::PoppingEmptyList>;

	namespace detail::adl {

	inline auto to_display_string(ErrorsV::HashMapDuplicateKeyError const &)
	    -> StringView
	{
		return StringView("HashMap duplicate key");
	}

	inline auto to_display_string(ErrorsV::InvalidIndex const &) -> StringView
	{
		return StringView("Invalid index");
	}

	inline auto to_display_string(ErrorsV::OutOfMemory const &) -> StringView
	{
		return StringView("Out of memory");
	}

	inline auto to_display_string(ErrorsV::PoppingEmptyList const &)
	    -> StringView
	{
		return StringView("Popping empty list");
	}

	inline auto to_display_string(Errors const &error) -> StringView
	{
		StringView result { };
		error.visit(
		    [&](auto const &value) { result = to_display_string(value); });
		return result;
	}

	}

	template<typename E> [[noreturn]] auto panic_error(E const &error) -> void
	{
		using detail::adl::to_display_string;
		panic(to_display_string(error).data());
	}

	}
}
