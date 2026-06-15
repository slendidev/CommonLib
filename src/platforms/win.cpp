#define WIN32_LEAN_AND_MEAN
#include <cstring>
#include <windows.h>

extern "C" [[noreturn]] auto commonlib_platform_panic(char const *message)
    -> void
{
	if (message != nullptr) {
		::OutputDebugStringA(message);
		::OutputDebugStringA("\r\n");

		DWORD written { 0 };
		HANDLE err { ::GetStdHandle(STD_ERROR_HANDLE) };
		if (err != INVALID_HANDLE_VALUE && err != nullptr) {
			::WriteFile(err, message, static_cast<DWORD>(std::strlen(message)),
			    &written, nullptr);
			::WriteFile(err, "\r\n", 2, &written, nullptr);
		}
	}

	::ExitProcess(1);
}
