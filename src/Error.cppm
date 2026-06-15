export module CommonLib:Error;

export import :Errors;

import :Result;
import :String;
import :TypeTraits;
import :Utility;
import :Variant;

export {
	namespace CL {

	struct ErasedError;

	namespace detail {

	template<typename...> using VoidT = void;

	template<typename T> struct AlwaysFalse {
		static constexpr bool Value = false;
	};

	template<typename T> struct IsErasedError {
		static constexpr bool Value = false;
	};

	template<typename... Es> struct IsVariantError {
		static constexpr bool Value = false;
	};

	template<typename... Es> struct IsVariantError<Variant<Es...>> {
		static constexpr bool Value = true;
	};

	template<typename T, typename = void> struct HasDisplayString {
		static constexpr bool Value = false;
	};

	template<typename T>
	struct HasDisplayString<T,
	    VoidT<decltype(String(to_display_string(declval<T>())))>> {
		static constexpr bool Value = true;
	};

	template<typename T>
	inline constexpr bool HasDisplayStringV = HasDisplayString<T>::Value;

	template<typename T> auto erase_error_value(T &&value) -> ErasedError;
	template<typename VariantType, int I = 0>
	auto erase_error_variant(VariantType const &error) -> ErasedError;

	}

	struct ErasedError {
		ErasedError() = default;
		ErasedError(String message)
		    : m_message { move(message) }
		{
		}

		ErasedError(StringView const message)
		    : m_message { message }
		{
		}

		ErasedError(char const *message)
		    : m_message { message }
		{
		}

		static auto msg(StringView const message) -> ErasedError
		{
			return ErasedError(message);
		}

		template<typename E> static auto from(E &&error) -> ErasedError;

		constexpr auto message() const -> StringView
		{
			return m_message.view();
		}

	private:
		String m_message;
	};

	namespace detail {

	template<> struct IsErasedError<ErasedError> {
		static constexpr bool Value = true;
	};

	}

	template<typename E> auto ErasedError::from(E &&error) -> ErasedError
	{
		using ErrorType = RemoveConstRef<E>;

		if constexpr (detail::IsErasedError<ErrorType>::Value)
			return forward<E>(error);

		using detail::adl::to_display_string;
		return ErasedError(String(to_display_string(forward<E>(error))));
	}

	template<typename T> using Fallible = Result<T, ErasedError>;

	}
}
