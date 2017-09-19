#pragma once

#include "term_buffer.h"
#include "py_plugin.h"

template<class TermBufferBase = TermBuffer>
class PyTermBuffer : public PyPlugin<TermBufferBase> {
public:
    using TermBufferBase::TermBufferBase;
};
