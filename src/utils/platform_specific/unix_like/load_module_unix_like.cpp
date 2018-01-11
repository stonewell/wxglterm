#include "load_module.h"
#include <dlfcn.h>

Handle LoadDyModuleFromFile(const char * file_path)
{
    Handle h {
        dlopen(file_path, RTLD_LAZY | RTLD_LOCAL),
        [](void* module)
        {
            if (module)
                dlclose(module);
        }
    };

    return h;
}

void * GetDyProcAddress(Handle module, const char * func_name)
{
    return dlsym(module.get(), func_name);
}
