module;

export module CommonLib:StringView;

import :StringOps;
import :Types;
import :TypeTraits;
import :Iterator;
import :Option;
import :Span;
import :Platform;

export {
	namespace CL {

	/// @brief A Unicode code point.
	struct Rune {
		u32 value;

		constexpr explicit Rune(u32 cp)
		    : value(cp)
		{
		}

		template<typename T>
		requires IsIntegralV<T> constexpr explicit operator T() const noexcept
		{
			return static_cast<T>(value);
		}
	};

	/// @brief A non-owning view of a string of characters.
	/// @tparam CharTypeT The type of the characters in the string view.
	template<typename CharTypeT>
	struct BaseStringView
	    : detail::StringViewOps<BaseStringView<CharTypeT>, CharTypeT> {
		struct ByteIter : Iterator<ByteIter> {
			CharTypeT const *current { nullptr };
			CharTypeT const *end { nullptr };

			auto next() -> Option<CharTypeT const &>
			{
				if (current == end)
					return { };

				return *current++;
			}

			auto next_back() -> Option<CharTypeT const &>
			{
				if (current == end)
					return { };

				--end;
				return *end;
			}
		};

		struct RuneIter : Iterator<RuneIter> {
			CharTypeT const *current { nullptr };
			CharTypeT const *end { nullptr };

			auto next() -> Option<Rune>
			{
				if (current == end)
					return { };

				Rune rune { 0 };
				current = decode_rune(current, end, rune);
				return rune;
			}

			auto next_back() -> Option<Rune>
			{
				if (current == end)
					return { };

				Rune rune { 0 };
				end = decode_rune_back(current, end, rune);
				return rune;
			}
		};

		static constexpr auto length(CharTypeT const *c_str) -> usize
		{
			usize len { };
			while (c_str[len] != '\0')
				len++;

			return len;
		}

		static constexpr auto byte(CharTypeT ch) -> u8
		{
			return static_cast<u8>(ch);
		}

		static constexpr auto continuation(u8 b) -> bool
		{
			return (b & 0xc0) == 0x80;
		}

		static constexpr auto sequence_length(u8 lead) -> usize
		{
			if ((lead & 0x80) == 0)
				return 1;
			if ((lead & 0xe0) == 0xc0)
				return 2;
			if ((lead & 0xf0) == 0xe0)
				return 3;
			if ((lead & 0xf8) == 0xf0)
				return 4;
			return 0;
		}

		static constexpr auto decode_rune(CharTypeT const *data,
		    CharTypeT const *end, Rune &rune) -> CharTypeT const *
		{
			u8 lead = byte(*data);
			usize len = sequence_length(lead);

			if (len == 0 || data + len > end)
				panic("Invalid UTF-8 string");

			u32 cp { };
			if (len == 1) {
				cp = lead;
			} else if (len == 2) {
				u8 b1 = byte(data[1]);
				if (!continuation(b1))
					panic("Invalid UTF-8 string");

				cp = static_cast<u32>(lead & 0x1f) << 6;
				cp |= static_cast<u32>(b1 & 0x3f);

				if (cp < 0x80)
					panic("Invalid UTF-8 string");
			} else if (len == 3) {
				u8 b1 = byte(data[1]);
				u8 b2 = byte(data[2]);
				if (!continuation(b1) || !continuation(b2))
					panic("Invalid UTF-8 string");

				cp = static_cast<u32>(lead & 0x0f) << 12;
				cp |= static_cast<u32>(b1 & 0x3f) << 6;
				cp |= static_cast<u32>(b2 & 0x3f);

				if (cp < 0x800 || (cp >= 0xd800 && cp <= 0xdfff))
					panic("Invalid UTF-8 string");
			} else {
				u8 b1 = byte(data[1]);
				u8 b2 = byte(data[2]);
				u8 b3 = byte(data[3]);
				if (!continuation(b1) || !continuation(b2) || !continuation(b3))
					panic("Invalid UTF-8 string");

				cp = static_cast<u32>(lead & 0x07) << 18;
				cp |= static_cast<u32>(b1 & 0x3f) << 12;
				cp |= static_cast<u32>(b2 & 0x3f) << 6;
				cp |= static_cast<u32>(b3 & 0x3f);

				if (cp < 0x10000 || cp > 0x10ffff)
					panic("Invalid UTF-8 string");
			}

			rune = Rune { cp };
			return data + len;
		}

		static constexpr auto decode_rune_back(CharTypeT const *begin,
		    CharTypeT const *end, Rune &rune) -> CharTypeT const *
		{
			CharTypeT const *start = end;
			do {
				--start;
			} while (start != begin && continuation(byte(*start)));

			decode_rune(start, end, rune);
			return start;
		}

		static constexpr auto validate_utf8(CharTypeT const *data, usize size)
		    -> void
		{
			usize index { };
			while (index < size) {
				Rune rune { 0 };
				index = static_cast<usize>(
				    decode_rune(data + index, data + size, rune) - data);
			}
		}

		constexpr BaseStringView()
		    : BaseStringView { "" }
		{
		}

		constexpr BaseStringView(CharTypeT const *c_str)
		    : m_data { c_str }
		    , m_size { length(c_str) }
		{
			validate_utf8(m_data, m_size);
		}

		constexpr BaseStringView(CharTypeT const *data, usize size)
		    : m_data { data }
		    , m_size { size }
		{
			validate_utf8(m_data, m_size);
		}

		/// @brief Get a pointer to the characters in the string view.
		/// @return A pointer to the characters in the string view.
		constexpr auto data() -> CharTypeT const * { return m_data; }
		/// @brief Get a pointer to the characters in the string view.
		/// @return A pointer to the characters in the string view.
		constexpr auto data() const -> CharTypeT const * { return m_data; }

		/// @brief Get the size of the string view.
		/// @return The size of the string view.
		constexpr auto size() -> usize { return m_size; }
		/// @brief Get the size of the string view.
		/// @return The size of the string view.
		constexpr auto size() const -> usize { return m_size; }

		/// @brief Get an iterator over the bytes in the string view.
		/// @return An iterator over the bytes in the string view.
		constexpr auto iter_bytes() const -> ByteIter
		{
			return ByteIter {
				.current = data(),
				.end = data() + size(),
			};
		}

		/// @brief Get an iterator over the runes in the string view.
		/// @return An iterator over the runes in the string view.
		constexpr auto iter() const -> RuneIter
		{
			return RuneIter {
				.current = data(),
				.end = data() + size(),
			};
		}

	private:
		CharTypeT const *m_data { nullptr };
		usize m_size { };
	};

	using StringView = BaseStringView<char>;

	}
}
