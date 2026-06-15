export module CommonLib:Platform;

extern "C" [[noreturn]] void commonlib_platform_panic(char const *message);

export {
	namespace CL {

	[[noreturn]] inline void panic(char const *message)
	{
		commonlib_platform_panic(message);
	}

	}
}
