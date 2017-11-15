#pragma once

#include "py_multiple_instance_plugin.h"
#include "term_data_handler.h"

template<class TermDataHandlerBase = TermDataHandler>
class PyTermDataHandler : public virtual PyMultipleInstancePlugin<TermDataHandlerBase> {
public:
    void OnData(const std::vector<unsigned char> & data, size_t data_len) override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermDataHandlerBase, "on_data", OnData, data, data_len);
    }
    void Start() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermDataHandlerBase, "start", Start, );
    }
    void Stop() override {
        PYBIND11_OVERLOAD_PURE_NAME(void, TermDataHandlerBase, "stop", Stop, );
    }
};
