#pragma once

#include <wx/wx.h>
#include <memory>

class wxGLTermApp : public wxApp
{
public:
    virtual bool OnInit();

    virtual int OnExit();
private:
    std::shared_ptr<void> m_PyInterpreter;
};
