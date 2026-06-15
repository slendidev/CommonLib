module;

#include <new>

export module CommonLib:Rc;

import :Errors;
import :Platform;
import :Types;
import :Result;
import :TypeTraits;
import :Utility;

export {
	namespace CL {

	/// @brief A reference-counted smart pointer for managing shared ownership
	/// of a value of type T.
	/// @tparam T The type of the value being managed by the Rc.
	template<typename T> struct Rc {
		struct RawTag { };

		template<typename... Args>
		requires(!(sizeof...(Args) == 1
		    && (SameAs<RemoveConstRef<Args>, Rc<T>> || ...)))
		explicit Rc(Args &&...args)
		{
			auto result { make(forward<Args>(args)...) };
			if (result.is_err())
				panic_error(result.unwrap_err());

			*this = move(result.unwrap());
		}

		static auto make() -> Result<Rc, Errors>
		{
			return Result<Rc, Errors>::Ok(Rc(RawTag { }));
		}

		template<typename... Args>
		requires(!(sizeof...(Args) == 1
		    && (SameAs<RemoveConstRef<Args>, Rc<T>> || ...)))
		static auto make(Args &&...args) -> Result<Rc, Errors>
		{
			void *ptr_mem = ::operator new(sizeof(T), std::nothrow);
			if (ptr_mem == nullptr)
				return Result<Rc, Errors>::Err(ErrorsV::OutOfMemory { });

			T *ptr = new (ptr_mem) T(forward<Args>(args)...);

			void *refs_mem = ::operator new(sizeof(usize), std::nothrow);
			if (refs_mem == nullptr) {
				::operator delete(ptr_mem);
				return Result<Rc, Errors>::Err(ErrorsV::OutOfMemory { });
			}

			usize *refs = new (refs_mem) usize(1);

			Rc rc(RawTag { });
			rc.m_ptr = ptr;
			rc.m_refs = refs;
			return Result<Rc, Errors>::Ok(move(rc));
		}

		~Rc() { release(); }

		Rc(Rc const &other)
		    : m_ptr { other.m_ptr }
		    , m_refs { other.m_refs }
		{
			if (m_refs)
				++(*m_refs);
		}

		Rc &operator=(Rc const &other)
		{
			if (this != &other) {
				release();

				m_ptr = other.m_ptr;
				m_refs = other.m_refs;

				if (m_refs)
					++(*m_refs);
			}

			return *this;
		}

		Rc(Rc &&other) noexcept
		    : m_ptr { other.m_ptr }
		    , m_refs { other.m_refs }
		{
			other.m_ptr = nullptr;
			other.m_refs = nullptr;
		}

		Rc &operator=(Rc &&other) noexcept
		{
			if (this != &other) {
				release();

				m_ptr = other.m_ptr;
				m_refs = other.m_refs;

				other.m_ptr = nullptr;
				other.m_refs = nullptr;
			}

			return *this;
		}

		T &operator*() { return *m_ptr; }
		T const &operator*() const { return *m_ptr; }

		T *operator->() { return m_ptr; }
		T const *operator->() const { return m_ptr; }

		// @brief Get a pointer to the managed value.
		// @return A pointer to the managed value.
		T *get() { return m_ptr; }
		T const *get() const { return m_ptr; }

		/// @brief Get the number of references to the managed value.
		/// @return The number of references to the managed value.
		auto ref_count() const -> usize { return m_refs ? *m_refs : 0; }

	private:
		explicit Rc(RawTag) { }

		auto release() -> void
		{
			if (!m_refs)
				return;

			--(*m_refs);

			if (*m_refs == 0) {
				delete m_ptr;
				delete m_refs;
			}

			m_ptr = nullptr;
			m_refs = nullptr;
		}

	private:
		T *m_ptr { nullptr };
		usize *m_refs { nullptr };
	};

	}
}
