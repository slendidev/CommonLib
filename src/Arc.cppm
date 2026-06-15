module;

#include <new>

export module CommonLib:Arc;

import :Errors;
import :Atomic;
import :Platform;
import :Types;
import :Result;
import :TypeTraits;
import :Utility;

export {
	namespace CL {

	/// @brief An atomically reference-counted smart pointer that owns a
	/// heap-allocated object of type T.
	/// @tparam T The type of the object owned by the Arc.
	template<typename T> struct Arc {
		template<typename... Args>
		requires(!(sizeof...(Args) == 1
		    && (SameAs<RemoveConstRef<Args>, Arc<T>> || ...)))
		explicit Arc(Args &&...args)
		{
			auto result { make(forward<Args>(args)...) };
			if (result.is_err())
				panic_error(result.unwrap_err());

			*this = move(result.unwrap());
		}

		static auto make() -> Result<Arc, Errors>
		{
			return Result<Arc, Errors>::Ok(Arc(RawTag { }));
		}

		template<typename... Args>
		requires(!(sizeof...(Args) == 1
		    && (SameAs<RemoveConstRef<Args>, Arc<T>> || ...)))
		static auto make(Args &&...args) -> Result<Arc, Errors>
		{
			void *ptr_mem = ::operator new(sizeof(T), std::nothrow);
			if (ptr_mem == nullptr)
				return Result<Arc, Errors>::Err(ErrorsV::OutOfMemory { });

			T *ptr = new (ptr_mem) T(forward<Args>(args)...);

			void *refs_mem
			    = ::operator new(sizeof(Atomic<usize>), std::nothrow);
			if (refs_mem == nullptr) {
				::operator delete(ptr_mem);
				return Result<Arc, Errors>::Err(ErrorsV::OutOfMemory { });
			}

			Atomic<usize> *refs = new (refs_mem) Atomic<usize>(1);

			Arc arc(RawTag { });
			arc.m_ptr = ptr;
			arc.m_refs = refs;
			return Result<Arc, Errors>::Ok(move(arc));
		}

		~Arc() { release(); }

		Arc(Arc const &other)
		    : m_ptr { other.m_ptr }
		    , m_refs { other.m_refs }
		{
			if (m_refs)
				m_refs->fetch_add(1, MemoryOrder::Relaxed);
		}

		Arc &operator=(Arc const &other)
		{
			if (this != &other) {
				release();

				m_ptr = other.m_ptr;
				m_refs = other.m_refs;

				if (m_refs)
					m_refs->fetch_add(1, MemoryOrder::Relaxed);
			}

			return *this;
		}

		Arc(Arc &&other) noexcept
		    : m_ptr { other.m_ptr }
		    , m_refs { other.m_refs }
		{
			other.m_ptr = nullptr;
			other.m_refs = nullptr;
		}

		Arc &operator=(Arc &&other) noexcept
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

		/// @brief Get a pointer to the owned object.
		/// @return A pointer to the owned object.
		T *get() { return m_ptr; }
		/// @brief Get a pointer to the owned object.
		/// @return A pointer to the owned object.
		T const *get() const { return m_ptr; }

		auto ref_count() const -> usize
		{
			return m_refs ? m_refs->load(MemoryOrder::Relaxed) : 0;
		}

	private:
		struct RawTag { };

		explicit Arc(RawTag) { }

		auto release() -> void
		{
			if (!m_refs)
				return;

			if (m_refs->fetch_sub(1, MemoryOrder::AcquireRelease) == 1) {
				delete m_ptr;
				delete m_refs;
			}

			m_ptr = nullptr;
			m_refs = nullptr;
		}

	private:
		T *m_ptr { nullptr };
		Atomic<usize> *m_refs { nullptr };
	};

	}
}
