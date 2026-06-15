export module CommonLib:Span;

import :Iterator;
import :Option;
import :Types;

export {
	namespace CL {

	/// @brief A non-owning view over a contiguous sequence of elements, similar
	/// to std::span in C++20.
	/// @tparam T The type of elements in the span.
	template<typename T> struct Span {
		struct Iter : Iterator<Iter> {
			T *current { nullptr };
			T *end { nullptr };

			auto next() -> Option<T &>
			{
				if (current == end)
					return { };

				return *current++;
			}

			auto next_back() -> Option<T &>
			{
				if (current == end)
					return { };

				--end;
				return *end;
			}
		};

		struct ConstIter : Iterator<ConstIter> {
			T const *current { nullptr };
			T const *end { nullptr };

			auto next() -> Option<T const &>
			{
				if (current == end)
					return { };

				return *current++;
			}

			auto next_back() -> Option<T const &>
			{
				if (current == end)
					return { };

				--end;
				return *end;
			}
		};

		constexpr Span() = default;

		explicit Span(T *data, usize size)
		    : m_data { data }
		    , m_size { size }
		{
		}
		Span(Span const &) = default;
		Span(Span &&) noexcept = default;
		auto operator=(Span const &) -> Span & = default;
		auto operator=(Span &&) noexcept -> Span & = default;
		~Span() = default;

		/// @brief Returns a pointer to the underlying data of the span.
		/// @return A pointer to the first element in the span.
		constexpr auto data() -> T * { return m_data; }
		/// @brief Returns a pointer to the underlying data of the span.
		/// @return A pointer to the first element in the span.
		constexpr auto data() const -> T const * { return m_data; }

		/// @brief Returns the number of elements in the span.
		/// @return The number of elements in the span.
		constexpr auto size() -> usize { return m_size; }
		/// @brief Returns the number of elements in the span.
		/// @return The number of elements in the span.
		constexpr auto size() const -> usize { return m_size; }

		/// @brief Checks if the span is empty (i.e., contains no elements).
		/// @return true if the span is empty, false otherwise.
		constexpr auto empty() const -> bool { return m_size == 0; }

		/// @brief Returns an iterator to the beginning of the span.
		/// @return An iterator to the first element in the span.
		auto iter() -> Iter
		{
			return Iter { .current = m_data, .end = m_data + m_size };
		}
		/// @brief Returns a constant iterator to the beginning of the span.
		/// @return A constant iterator to the first element in the span.
		auto iter() const -> ConstIter
		{
			return ConstIter { .current = m_data, .end = m_data + m_size };
		}

		constexpr auto operator[](usize index) -> T & { return m_data[index]; }
		constexpr auto operator[](usize index) const -> T const &
		{
			return m_data[index];
		}

	private:
		T *m_data { nullptr };
		usize m_size { };
	};

	}
}
