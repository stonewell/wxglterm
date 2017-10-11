#pragma once

#include <memory>

#define DECL_SHARED_PTR(class_name) \
    class class_name; \
    using class_name##Ptr = std::shared_ptr<class_name>;

DECL_SHARED_PTR(Plugin);
DECL_SHARED_PTR(MultipleInstancePlugin);
DECL_SHARED_PTR(Context);
DECL_SHARED_PTR(PluginManager);

DECL_SHARED_PTR(TermContext);
DECL_SHARED_PTR(TermUI);
DECL_SHARED_PTR(TermLine);
DECL_SHARED_PTR(TermNetwork);
DECL_SHARED_PTR(TermCell);
DECL_SHARED_PTR(TermBuffer);
DECL_SHARED_PTR(AppConfig);
DECL_SHARED_PTR(TermDataHandler);
DECL_SHARED_PTR(TermWindow);
DECL_SHARED_PTR(Task);
DECL_SHARED_PTR(TermSelection);
