module;

#include <initializer_list>

export module CommonLib:InitializerList;

import :Types;

export {
	namespace CL {
	template<typename T> using InitializerList = std::initializer_list<T>;
	}
}
