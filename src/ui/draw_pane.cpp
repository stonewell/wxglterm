#include "draw_pane.h"

BEGIN_EVENT_TABLE(DrawPane, wxPanel)
EVT_PAINT(DrawPane::PaintEvent)
END_EVENT_TABLE()

DrawPane::DrawPane(wxFrame * parent)
    : wxPanel(parent)
    , m_RefreshNow(0)
{
    Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
}

DrawPane::~DrawPane()
{
    Disconnect( wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
}

void DrawPane::Refresh()
{
    wxCriticalSectionLocker locker(m_RefreshLock);
    m_RefreshNow++;
}

void DrawPane::PaintEvent(wxPaintEvent & /*event*/)
{
    wxPaintDC dc(this);

    DoPaint(dc);
}

void DrawPane::DoPaint(wxDC & dc)
{
    static int y = 0;
    static int y_speed = 2;

    y += y_speed;
    if(y<0) y_speed = 2;
    if(y>200) y_speed = -2;

    dc.SetBackground( *wxWHITE_BRUSH );
    dc.Clear();

    dc.DrawText(wxString::Format("Test y=%d", y), 40, y);
}

void DrawPane::OnIdle(wxIdleEvent& evt)
{
    int refreshNow = 0;

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        refreshNow = m_RefreshNow;
    }

    if (refreshNow)
    {
        wxClientDC dc(this);
        DoPaint(dc);
    }

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        m_RefreshNow -= refreshNow;
    }

    evt.RequestMore(); // render continuously, not only once on idle
}
