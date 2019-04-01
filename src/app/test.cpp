#include <windows.h>
#include <iostream>

typedef int (*pfn_test_main)();

int test_main() {
    HMODULE h = LoadLibrary("a.dll");

    pfn_test_main pfn = (pfn_test_main)GetProcAddress(h, "test_main");

    std::cout << "find the function:" << pfn << std::endl;

    if (pfn)
        return pfn();

    return 0;
}
