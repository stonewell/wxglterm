#pragma once

class __ScopeLocker {
public:
    __ScopeLocker(TermBufferPtr termBuffer) :
        m_TermBuffer(termBuffer)
    {
        m_TermBuffer->LockUpdate();
    }

    ~__ScopeLocker() {
        m_TermBuffer->UnlockUpdate();
    }

    TermBufferPtr m_TermBuffer;
};
