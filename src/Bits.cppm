export module CommonLib:Bits;

import :TypeTraits;
import :Types;

export namespace CL {

template<typename T>
requires(IsUnsignedIntegralV<T> && !IsSameV<T, bool>)
constexpr auto little_endian(u8 const *data) -> T
{
	T value { };

	for (usize i { sizeof(T) }; i > 0; --i)
		value = static_cast<T>((value << 8) | data[i - 1]);

	return value;
}

}
