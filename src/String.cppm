export module CommonLib:String;

import :ArrayList;
import :Errors;
import :Iterator;
import :StringOps;
import :StringView;
import :Result;
import :Platform;
import :Types;
import :UtilityBase;

export {
	namespace CL {

	/// @brief An owning string of characters.
	/// @tparam CharTypeT The type of the characters in the string.
	template<typename CharTypeT>
	struct BaseString : detail::StringOps<BaseString<CharTypeT>, CharTypeT> {
		struct RawTag { };

		struct ByteIter : Iterator<ByteIter> {
			typename ArrayList<CharTypeT>::Iter iter;
			usize remaining;

			auto next() -> Option<CharTypeT &>
			{
				if (remaining == 0)
					return { };

				remaining--;
				return iter.next();
			}

			auto next_back() -> Option<CharTypeT &>
			{
				if (remaining == 0)
					return { };

				remaining--;
				return iter.next_back();
			}
		};

		struct ConstByteIter : Iterator<ConstByteIter> {
			typename ArrayList<CharTypeT>::ConstIter iter;
			usize remaining;

			auto next() -> Option<CharTypeT const &>
			{
				if (remaining == 0)
					return { };

				remaining--;
				return iter.next();
			}

			auto next_back() -> Option<CharTypeT const &>
			{
				if (remaining == 0)
					return { };

				remaining--;
				return iter.next_back();
			}
		};

		BaseString()
		{
			auto result { make() };
			if (result.is_err())
				panic_error(result.unwrap_err());

			*this = move(result.unwrap());
		}

		BaseString(CharTypeT const *cstring)
		{
			auto result { make(cstring) };
			if (result.is_err())
				panic_error(result.unwrap_err());

			*this = move(result.unwrap());
		}

		explicit BaseString(BaseStringView<CharTypeT> const &string)
		{
			auto result { make(string) };
			if (result.is_err())
				panic_error(result.unwrap_err());

			*this = move(result.unwrap());
		}

		static auto make() -> Result<BaseString, Errors>
		{
			return make(BaseStringView<CharTypeT> { });
		}

		static auto make(CharTypeT const *cstring) -> Result<BaseString, Errors>
		{
			return make(BaseStringView<CharTypeT> { cstring });
		}

		static auto make(BaseStringView<CharTypeT> const &string)
		    -> Result<BaseString, Errors>
		{
			auto data_result { ArrayList<CharTypeT>::make(string.size() + 1) };
			if (data_result.is_err())
				return Result<BaseString, Errors>::Err(
				    data_result.unwrap_err());

			BaseString result { RawTag { } };
			result.m_data = move(data_result.unwrap());

			for (usize i { }; i < string.size(); ++i)
				result.m_data.push(string.data()[i]);

			result.m_data.push('\0');

			return Result<BaseString, Errors>::Ok(move(result));
		}

		/// @brief Get an iterator over the bytes in the string.
		/// @return An iterator over the bytes in the string.
		auto iter_bytes() -> ByteIter
		{
			return ByteIter {
				.iter = m_data.iter(),
				.remaining = size(),
			};
		}

		/// @brief Get an iterator over the bytes in the string.
		/// @return An iterator over the bytes in the string.
		auto iter_bytes() const -> ConstByteIter
		{
			return ConstByteIter {
				.iter = m_data.iter(),
				.remaining = size(),
			};
		}

		/// @brief Get an iterator over the runes in the string.
		/// @return An iterator over the runes in the string.
		auto iter() -> typename BaseStringView<CharTypeT>::RuneIter
		{
			return BaseStringView<CharTypeT>(data(), size()).iter();
		}

		/// @brief Get an iterator over the runes in the string.
		/// @return An iterator over the runes in the string.
		auto iter() const -> typename BaseStringView<CharTypeT>::RuneIter
		{
			return BaseStringView<CharTypeT>(data(), size()).iter();
		}

		/// @brief Get the size of the string.
		/// @return The size of the string.
		constexpr auto size() const -> usize
		{
			return m_data.size() ? m_data.size() - 1 : 0;
		}

		/// @brief Get the current capacity of the string buffer.
		/// @return The number of characters that can be stored without
		/// reallocation.
		constexpr auto capacity() const -> usize
		{
			usize buffer_capacity = m_data.capacity();
			return buffer_capacity ? buffer_capacity - 1 : 0;
		}

		/// @brief Get a pointer to the characters in the string.
		/// @return A pointer to the characters in the string.
		constexpr auto data() -> CharTypeT * { return m_data.data(); }
		/// @brief Get a pointer to the characters in the string.
		/// @return A pointer to the characters in the string.
		constexpr auto data() const -> CharTypeT const *
		{
			return m_data.data();
		}

		/// @brief Get a null-terminated C string.
		/// @return A null-terminated C string.
		constexpr auto c_str() const -> CharTypeT const *
		{
			return m_data.data();
		}

		/// @brief Clear the string, making it empty.
		auto clear() -> void
		{
			m_data.clear();
			m_data.push('\0');
		}

		/// @brief Append a character to the end of the string.
		/// @param ch The character to append.
		auto append(CharTypeT ch) -> void
		{
			m_data.pop();
			m_data.push(ch);
			m_data.push('\0');
		}

		/// @brief Append a string to the end of the string.
		/// @param string The string to append.
		auto append(BaseStringView<CharTypeT> const &string) -> void
		{
			m_data.pop();
			auto reserve_result { m_data.reserve(size() + string.size() + 1) };
			if (reserve_result.is_err())
				panic_error(reserve_result.unwrap_err());

			for (usize i { }; i < string.size(); ++i)
				m_data.push(string.data()[i]);

			m_data.push('\0');
		}

		auto operator+=(CharTypeT ch) -> BaseString &
		{
			append(ch);
			return *this;
		}

		auto operator+=(BaseStringView<CharTypeT> const &string) -> BaseString &
		{
			append(string);
			return *this;
		}

		constexpr auto operator[](usize index) -> CharTypeT &
		{
			return m_data[index];
		}

		constexpr auto operator[](usize index) const -> CharTypeT const &
		{
			return m_data[index];
		}

	private:
		explicit BaseString(RawTag) { }

		ArrayList<CharTypeT> m_data;
	};

	template<typename CharTypeT>
	auto operator+(BaseString<CharTypeT> const &lhs,
	    BaseStringView<CharTypeT> const &rhs) -> BaseString<CharTypeT>
	{
		BaseString<CharTypeT> result(lhs.view());
		result += rhs;
		return result;
	}

	using String = BaseString<char>;

	}
}
