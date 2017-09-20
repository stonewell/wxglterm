#pragma once

#include <wx/wx.h>
#include <memory>

class BatchRenameApp : public wxApp
{
  public:
    virtual bool OnInit();

private:
    std::shared_ptr<void> guard;
};
