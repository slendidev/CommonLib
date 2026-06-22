#include <cstdlib>
#include <cstring>

#include <unistd.h>

extern "C" [[noreturn]] auto commonlib_platform_panic(char const *message)
    -> void
{
	if (message != nullptr) {
		::write(STDERR_FILENO, message, strlen(message));
		::write(STDERR_FILENO, "\n", 1);
	}

	std::abort();
}
