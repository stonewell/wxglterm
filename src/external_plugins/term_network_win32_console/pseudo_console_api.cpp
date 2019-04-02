#include "pseudo_console_api.h"
#include <iostream>

typedef HRESULT (*pfn_CreatePseudoConsole)(COORD size, HANDLE hInput, HANDLE hOutput, DWORD dwFlags, HPCON * phPC);
typedef HRESULT (*pfn_ResizePseudoConsole)(HPCON hPC, COORD size);
typedef HRESULT (*pfn_ClosePseudoConsole)(HPCON hPC);
typedef BOOL (*pfn_InitializeProcThreadAttributeList)(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD                        dwAttributeCount,
    DWORD                        dwFlags,
    PSIZE_T                      lpSize
                                                      );
typedef BOOL (*pfn_UpdateProcThreadAttribute)(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD                        dwFlags,
    DWORD_PTR                    Attribute,
    PVOID                        lpValue,
    SIZE_T                       cbSize,
    PVOID                        lpPreviousValue,
    PSIZE_T                      lpReturnSize
                                              );
typedef void (*pfn_DeleteProcThreadAttributeList)(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList
                                                  );

class Kernel32Api {
public:
    pfn_CreatePseudoConsole fn_create;
    pfn_ResizePseudoConsole fn_resize;
    pfn_ClosePseudoConsole fn_close;

    pfn_InitializeProcThreadAttributeList fn_init_thread_attr_list;
    pfn_UpdateProcThreadAttribute fn_update_thread_attr;
    pfn_DeleteProcThreadAttributeList fn_delete_thread_attr_list;

    HMODULE hKernel32;
public:
    Kernel32Api() {
        hKernel32 = LoadLibrary("kernel32");
        fn_create = NULL;
        fn_resize = NULL;
        fn_close = NULL;
        fn_init_thread_attr_list = NULL;
        fn_update_thread_attr = NULL;
        fn_delete_thread_attr_list = NULL;

        if (hKernel32 != NULL) {
            fn_create = (pfn_CreatePseudoConsole)GetProcAddress(hKernel32, "CreatePseudoConsole");
            fn_resize = (pfn_ResizePseudoConsole)GetProcAddress(hKernel32, "ResizePseudoConsole");
            fn_close = (pfn_ClosePseudoConsole)GetProcAddress(hKernel32, "ClosePseudoConsole");

            fn_init_thread_attr_list = (pfn_InitializeProcThreadAttributeList)GetProcAddress(hKernel32, "InitializeProcThreadAttributeList");
            fn_update_thread_attr = (pfn_UpdateProcThreadAttribute)GetProcAddress(hKernel32, "UpdateProcThreadAttribute");
            fn_delete_thread_attr_list = (pfn_DeleteProcThreadAttributeList)GetProcAddress(hKernel32, "DeleteProcThreadAttributeList");
        }
    }

    ~Kernel32Api() {
        if (hKernel32 != NULL)
            FreeLibrary(hKernel32);
    }
};

static
Kernel32Api g_Kernel32Api{};

HRESULT CreatePseudoConsole(COORD size, HANDLE hInput, HANDLE hOutput, DWORD dwFlags, HPCON * phPC) {
    if (g_Kernel32Api.fn_create) {
         return g_Kernel32Api.fn_create(size, hInput, hOutput, dwFlags, phPC);
    }
    else
        std::cerr << "CreatePseudoConsole is not found" << std::endl;

    return E_FAIL;
}

HRESULT ResizePseudoConsole(HPCON hPC, COORD size) {
    if (g_Kernel32Api.fn_resize)
        return g_Kernel32Api.fn_resize(hPC, size);
    else
        std::cerr << "ResizePseudoConsole is not found" << std::endl;

    return E_FAIL;
}

HRESULT ClosePseudoConsole(HPCON hPC) {
    if (g_Kernel32Api.fn_close)
        return g_Kernel32Api.fn_close(hPC);
    else
        std::cerr << "ClosePseudoConsole is not found" << std::endl;

    return E_FAIL;
}

#if _WIN32_WINNT < 0x0600
BOOL InitializeProcThreadAttributeList(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD                        dwAttributeCount,
    DWORD                        dwFlags,
    PSIZE_T                      lpSize
                                       ) {
    if (g_Kernel32Api.fn_init_thread_attr_list)
        return g_Kernel32Api.fn_init_thread_attr_list(lpAttributeList, dwAttributeCount, dwFlags, lpSize);
    else
        std::cerr << "InitializeProcThreadAttributeList is not found" << std::endl;

    return FALSE;
}

BOOL UpdateProcThreadAttribute(
    LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
    DWORD                        dwFlags,
    DWORD_PTR                    Attribute,
    PVOID                        lpValue,
    SIZE_T                       cbSize,
    PVOID                        lpPreviousValue,
    PSIZE_T                      lpReturnSize
                               ) {
    if (g_Kernel32Api.fn_update_thread_attr)
        return g_Kernel32Api.fn_update_thread_attr(lpAttributeList, dwFlags, Attribute, lpValue, cbSize, lpPreviousValue, lpReturnSize);
    else
        std::cerr << "UpdateProcThreadAttribute is not found" << std::endl;

    return FALSE;
}

void DeleteProcThreadAttributeList(
  LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList
                                   ) {
    if (g_Kernel32Api.fn_delete_thread_attr_list)
        g_Kernel32Api.fn_delete_thread_attr_list(lpAttributeList);
    else
        std::cerr << "DeleteProcThreadAttributeList is not found" << std::endl;
}
#endif
