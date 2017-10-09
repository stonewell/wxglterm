#pragma once

#include "py_plugin.h"
#include "task.h"

template<class TaskBase = Task>
class PyTask : public virtual PyPlugin<TaskBase> {
public:
    using PyPlugin<TaskBase>::PyPlugin;

    void Run() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TaskBase, "run", Run, );
    }

    void Cancel() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TaskBase, "cancel", Cancel, );
    }

    bool IsCancelled() override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, TaskBase, "is_cancelled", IsCancelled, );
    }
};
