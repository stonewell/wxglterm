#pragma once

#include "py_multiple_instance_plugin.h"
#include "context.h"

template<class ContextBase = Context>
class PyContext : public virtual PyMultipleInstancePlugin<ContextBase> {
public:
    using PyMultipleInstancePlugin<ContextBase>::PyMultipleInstancePlugin;

public:

};
