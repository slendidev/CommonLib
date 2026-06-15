module;

#if defined(_MSC_VER)
#	include <intrin.h>
#endif

export module CommonLib:Atomic;

export {
	namespace CL {

/// @brief Memory order for atomic operations.
#if defined(_MSC_VER)
	enum class MemoryOrder {
		/// @brief No ordering constraints.
		Relaxed,
		/// @brief Acquire ordering constraints.
		Acquire,
		/// @brief Release ordering constraints.
		Release,
		/// @brief Acquire-release ordering constraints.
		AcquireRelease,
		/// @brief Sequentially consistent ordering constraints.
		SequentiallyConsistant,
	};
#else
	enum class MemoryOrder {
		/// @brief No ordering constraints.
		Relaxed = __ATOMIC_RELAXED,
		/// @brief Acquire ordering constraints.
		Acquire = __ATOMIC_ACQUIRE,
		/// @brief Release ordering constraints.
		Release = __ATOMIC_RELEASE,
		/// @brief Acquire-release ordering constraints.
		AcquireRelease = __ATOMIC_ACQ_REL,
		/// @brief Sequentially consistent ordering constraints.
		SequentiallyConsistant = __ATOMIC_SEQ_CST,
	};
#endif

	/// @brief An atomic variable of type T.
	/// @tparam T The type of the atomic variable.
	template<typename T> struct Atomic {
		Atomic() = default;

		constexpr Atomic(T value)
		    : m_value { value }
		{
		}

		Atomic(Atomic const &) = delete;
		Atomic &operator=(Atomic const &) = delete;

		/// @brief Load the value of the atomic variable.
		/// @param order The memory order for the load operation.
		/// @return The value of the atomic variable.
		auto load(MemoryOrder order = MemoryOrder::SequentiallyConsistant) const
		    -> T
		{
			(void)order;

#if defined(_MSC_VER)
			return load_impl();
#else
			return __atomic_load_n(&m_value, (int)order);
#endif
		}

		/// @brief Store a value to the atomic variable.
		/// @param value The value to store.
		/// @param order The memory order for the store operation.
		auto store(T value,
		    MemoryOrder order = MemoryOrder::SequentiallyConsistant) -> void
		{
			(void)order;

#if defined(_MSC_VER)
			store_impl(value);
#else
			__atomic_store_n(&m_value, value, (int)order);
#endif
		}

		auto exchange(T value,
		    MemoryOrder order = MemoryOrder::SequentiallyConsistant) -> T
		{
			(void)order;

#if defined(_MSC_VER)
			return exchange_impl(value);
#else
			return __atomic_exchange_n(&m_value, value, (int)order);
#endif
		}

		/// @brief Compare the value of the atomic variable with an expected
		/// value and exchange it if they match.
		/// @param expected A reference to the expected value.
		/// @param desired The value to exchange if the comparison succeeds.
		/// @param success The memory order for the operation if the comparison
		/// succeeds.
		/// @param failure The memory order for the operation if the comparison
		/// fails.
		/// @return True if the comparison succeeded and the exchange was
		/// performed, false otherwise.
		auto compare_exchange(T &expected, T desired,
		    MemoryOrder success = MemoryOrder::SequentiallyConsistant,
		    MemoryOrder failure = MemoryOrder::SequentiallyConsistant) -> bool
		{
			(void)success;
			(void)failure;

#if defined(_MSC_VER)
			return compare_exchange_impl(expected, desired);
#else
			return __atomic_compare_exchange_n(&m_value, &expected, desired,
			    false, (int)success, (int)failure);
#endif
		}

		/// @brief Atomically add a value to the atomic variable and return the
		/// previous value.
		/// @param value The value to add.
		/// @param order The memory order for the operation.
		/// @return The previous value of the atomic variable.
		auto fetch_add(T value,
		    MemoryOrder order = MemoryOrder::SequentiallyConsistant) -> T
		{
			(void)order;

#if defined(_MSC_VER)
			return fetch_add_impl(value);
#else
			return __atomic_fetch_add(&m_value, value, (int)order);
#endif
		}

		/// @brief Atomically subtract a value from the atomic variable and
		/// return the previous value.
		/// @param value The value to subtract.
		/// @param order The memory order for the operation.
		/// @return The previous value of the atomic variable.
		auto fetch_sub(T value,
		    MemoryOrder order = MemoryOrder::SequentiallyConsistant) -> T
		{
			(void)order;

#if defined(_MSC_VER)
			return fetch_sub_impl(value);
#else
			return __atomic_fetch_sub(&m_value, value, (int)order);
#endif
		}

		/// @brief Atomically perform a bitwise OR operation on the atomic
		/// variable and return the previous value.
		/// @param value The value to OR with.
		/// @param order The memory order for the operation.
		/// @return The previous value of the atomic variable.
		auto fetch_or(T value,
		    MemoryOrder order = MemoryOrder::SequentiallyConsistant) -> T
		{
			(void)order;

#if defined(_MSC_VER)
			return fetch_or_impl(value);
#else
			return __atomic_fetch_or(&m_value, value, (int)order);
#endif
		}

		/// @brief Atomically perform a bitwise AND operation on the atomic
		/// variable and return the previous value.
		/// @param value The value to AND with.
		/// @param order The memory order for the operation.
		/// @return The previous value of the atomic variable.
		auto fetch_and(T value,
		    MemoryOrder order = MemoryOrder::SequentiallyConsistant) -> T
		{
			(void)order;

#if defined(_MSC_VER)
			return fetch_and_impl(value);
#else
			return __atomic_fetch_and(&m_value, value, (int)order);
#endif
		}

		/// @brief Atomically perform a bitwise XOR operation on the atomic
		/// variable and return the previous value.
		/// @param value The value to XOR with.
		/// @param order The memory order for the operation.
		/// @return The previous value of the atomic variable.
		auto fetch_xor(T value,
		    MemoryOrder order = MemoryOrder::SequentiallyConsistant) -> T
		{
			(void)order;

#if defined(_MSC_VER)
			return fetch_xor_impl(value);
#else
			return __atomic_fetch_xor(&m_value, value, (int)order);
#endif
		}

		operator T() const { return load(); }

		auto operator=(T value) -> T
		{
			store(value);
			return value;
		}

	private:
#if defined(_MSC_VER)
		static auto ptr(T *value)
		{
			return reinterpret_cast<T volatile *>(value);
		}

		static auto ptr(T const *value)
		{
			return reinterpret_cast<T volatile *>(const_cast<T *>(value));
		}

		auto load_impl() const -> T
		{
			if constexpr (sizeof(T) == 8) {
				return static_cast<T>(_InterlockedCompareExchange64(
				    reinterpret_cast<long long volatile *>(
				        const_cast<T *>(&m_value)),
				    0, 0));
			} else if constexpr (sizeof(T) == 4) {
				return static_cast<T>(_InterlockedCompareExchange(
				    reinterpret_cast<long volatile *>(
				        const_cast<T *>(&m_value)),
				    0, 0));
			} else if constexpr (sizeof(T) == 2) {
				return static_cast<T>(_InterlockedCompareExchange16(
				    reinterpret_cast<short volatile *>(
				        const_cast<T *>(&m_value)),
				    0, 0));
			} else {
				return static_cast<T>(_InterlockedCompareExchange8(
				    reinterpret_cast<char volatile *>(
				        const_cast<T *>(&m_value)),
				    0, 0));
			}
		}

		void store_impl(T value)
		{
			if constexpr (sizeof(T) == 8) {
				_InterlockedExchange64(reinterpret_cast<long long volatile *>(
				                           const_cast<T *>(&m_value)),
				    static_cast<long long>(value));
			} else if constexpr (sizeof(T) == 4) {
				_InterlockedExchange(reinterpret_cast<long volatile *>(
				                         const_cast<T *>(&m_value)),
				    static_cast<long>(value));
			} else if constexpr (sizeof(T) == 2) {
				_InterlockedExchange16(reinterpret_cast<short volatile *>(
				                           const_cast<T *>(&m_value)),
				    static_cast<short>(value));
			} else {
				_InterlockedExchange8(reinterpret_cast<char volatile *>(
				                          const_cast<T *>(&m_value)),
				    static_cast<char>(value));
			}
		}

		auto exchange_impl(T value) -> T
		{
			if constexpr (sizeof(T) == 8) {
				return static_cast<T>(_InterlockedExchange64(
				    reinterpret_cast<long long volatile *>(
				        const_cast<T *>(&m_value)),
				    static_cast<long long>(value)));
			} else if constexpr (sizeof(T) == 4) {
				return static_cast<T>(
				    _InterlockedExchange(reinterpret_cast<long volatile *>(
				                             const_cast<T *>(&m_value)),
				        static_cast<long>(value)));
			} else if constexpr (sizeof(T) == 2) {
				return static_cast<T>(
				    _InterlockedExchange16(reinterpret_cast<short volatile *>(
				                               const_cast<T *>(&m_value)),
				        static_cast<short>(value)));
			} else {
				return static_cast<T>(
				    _InterlockedExchange8(reinterpret_cast<char volatile *>(
				                              const_cast<T *>(&m_value)),
				        static_cast<char>(value)));
			}
		}

		auto compare_exchange_impl(T &expected, T desired) -> bool
		{
			if constexpr (sizeof(T) == 8) {
				auto old = _InterlockedCompareExchange64(
				    reinterpret_cast<long long volatile *>(
				        const_cast<T *>(&m_value)),
				    static_cast<long long>(desired),
				    static_cast<long long>(expected));
				if (old == static_cast<long long>(expected))
					return true;
				expected = static_cast<T>(old);
				return false;
			} else if constexpr (sizeof(T) == 4) {
				auto old = _InterlockedCompareExchange(
				    reinterpret_cast<long volatile *>(
				        const_cast<T *>(&m_value)),
				    static_cast<long>(desired), static_cast<long>(expected));
				if (old == static_cast<long>(expected))
					return true;
				expected = static_cast<T>(old);
				return false;
			} else if constexpr (sizeof(T) == 2) {
				auto old = _InterlockedCompareExchange16(
				    reinterpret_cast<short volatile *>(
				        const_cast<T *>(&m_value)),
				    static_cast<short>(desired), static_cast<short>(expected));
				if (old == static_cast<short>(expected))
					return true;
				expected = static_cast<T>(old);
				return false;
			} else {
				auto old = _InterlockedCompareExchange8(
				    reinterpret_cast<char volatile *>(
				        const_cast<T *>(&m_value)),
				    static_cast<char>(desired), static_cast<char>(expected));
				if (old == static_cast<char>(expected))
					return true;
				expected = static_cast<T>(old);
				return false;
			}
		}

		auto fetch_add_impl(T value) -> T
		{
			if constexpr (sizeof(T) == 8) {
				return static_cast<T>(_InterlockedExchangeAdd64(
				    reinterpret_cast<long long volatile *>(
				        const_cast<T *>(&m_value)),
				    static_cast<long long>(value)));
			} else if constexpr (sizeof(T) == 4) {
				return static_cast<T>(
				    _InterlockedExchangeAdd(reinterpret_cast<long volatile *>(
				                                const_cast<T *>(&m_value)),
				        static_cast<long>(value)));
			} else if constexpr (sizeof(T) == 2) {
				return static_cast<T>(_InterlockedExchangeAdd16(
				    reinterpret_cast<short volatile *>(
				        const_cast<T *>(&m_value)),
				    static_cast<short>(value)));
			} else {
				return static_cast<T>(
				    _InterlockedExchangeAdd8(reinterpret_cast<char volatile *>(
				                                 const_cast<T *>(&m_value)),
				        static_cast<char>(value)));
			}
		}

		auto fetch_sub_impl(T value) -> T
		{
			return fetch_add_impl(static_cast<T>(0 - value));
		}

		auto fetch_or_impl(T value) -> T
		{
			if constexpr (sizeof(T) == 8) {
				return static_cast<T>(
				    _InterlockedOr64(reinterpret_cast<long long volatile *>(
				                         const_cast<T *>(&m_value)),
				        static_cast<long long>(value)));
			} else if constexpr (sizeof(T) == 4) {
				return static_cast<T>(
				    _InterlockedOr(reinterpret_cast<long volatile *>(
				                       const_cast<T *>(&m_value)),
				        static_cast<long>(value)));
			} else if constexpr (sizeof(T) == 2) {
				return static_cast<T>(
				    _InterlockedOr16(reinterpret_cast<short volatile *>(
				                         const_cast<T *>(&m_value)),
				        static_cast<short>(value)));
			} else {
				return static_cast<T>(
				    _InterlockedOr8(reinterpret_cast<char volatile *>(
				                        const_cast<T *>(&m_value)),
				        static_cast<char>(value)));
			}
		}

		auto fetch_and_impl(T value) -> T
		{
			if constexpr (sizeof(T) == 8) {
				return static_cast<T>(
				    _InterlockedAnd64(reinterpret_cast<long long volatile *>(
				                          const_cast<T *>(&m_value)),
				        static_cast<long long>(value)));
			} else if constexpr (sizeof(T) == 4) {
				return static_cast<T>(
				    _InterlockedAnd(reinterpret_cast<long volatile *>(
				                        const_cast<T *>(&m_value)),
				        static_cast<long>(value)));
			} else if constexpr (sizeof(T) == 2) {
				return static_cast<T>(
				    _InterlockedAnd16(reinterpret_cast<short volatile *>(
				                          const_cast<T *>(&m_value)),
				        static_cast<short>(value)));
			} else {
				return static_cast<T>(
				    _InterlockedAnd8(reinterpret_cast<char volatile *>(
				                         const_cast<T *>(&m_value)),
				        static_cast<char>(value)));
			}
		}

		auto fetch_xor_impl(T value) -> T
		{
			if constexpr (sizeof(T) == 8) {
				return static_cast<T>(
				    _InterlockedXor64(reinterpret_cast<long long volatile *>(
				                          const_cast<T *>(&m_value)),
				        static_cast<long long>(value)));
			} else if constexpr (sizeof(T) == 4) {
				return static_cast<T>(
				    _InterlockedXor(reinterpret_cast<long volatile *>(
				                        const_cast<T *>(&m_value)),
				        static_cast<long>(value)));
			} else if constexpr (sizeof(T) == 2) {
				return static_cast<T>(
				    _InterlockedXor16(reinterpret_cast<short volatile *>(
				                          const_cast<T *>(&m_value)),
				        static_cast<short>(value)));
			} else {
				return static_cast<T>(
				    _InterlockedXor8(reinterpret_cast<char volatile *>(
				                         const_cast<T *>(&m_value)),
				        static_cast<char>(value)));
			}
		}
#endif

		mutable T m_value { };
	};

	}
}
