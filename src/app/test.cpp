#ifdef _WIN32
#include <windows.h>
#include <iostream>

typedef int (*pfn_test_main)();
#endif

int test_main() {
#ifdef _WIN32
    HMODULE h = LoadLibrary("a.dll");

    pfn_test_main pfn = (pfn_test_main)GetProcAddress(h, "test_main");

    std::cout << "find the function:" << pfn << std::endl;

    if (pfn)
        return pfn();
#endif

    return 0;
}
