#include "load_module.h"
#include <windows.h>

Handle LoadDyModuleFromFile(const char * file_path)
{
    Handle h {
        LoadLibrary(file_path),
        [](void* module)
        {
            FreeLibrary((HMODULE)module);
        }
    };

    return h;
}

void * GetDyProcAddress(Handle module, const char * func_name)
{
    return GetProcAddress((HMODULE)module.get(), func_name);
}
