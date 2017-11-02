#include <pybind11/embed.h>

#include <iostream>

#include "term_window.h"
#include "draw_pane.h"
#include <wx/dcbuffer.h>

#include "term_context.h"
#include "term_buffer.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_network.h"
#include "color_theme.h"
#include "scope_locker.h"

wxCoord PADDING = 5;

#define SINGLE_WIDTH_CHARACTERS         \
					" !\"#$%&'()*+,-./" \
					"0123456789" \
					":;<=>?@" \
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
					"[\\]^_`" \
					"abcdefghijklmnopqrstuvwxyz" \
					"{|}~"

#define TIMER_ID (-1)

wxDECLARE_EVENT(MY_REFRESH_EVENT, wxCommandEvent);
// this is a definition so can't be in a header
wxDEFINE_EVENT(MY_REFRESH_EVENT, wxCommandEvent);

BEGIN_EVENT_TABLE(DrawPane, wxPanel)
        EVT_PAINT(DrawPane::OnPaint)
        EVT_SIZE(DrawPane::OnSize)
        EVT_KEY_DOWN(DrawPane::OnKeyDown)
        EVT_KEY_UP(DrawPane::OnKeyUp)
        EVT_CHAR(DrawPane::OnChar)
        EVT_ERASE_BACKGROUND(DrawPane::OnEraseBackground)
EVT_TIMER(TIMER_ID, DrawPane::OnTimer)
EVT_COMMAND(wxID_ANY, MY_REFRESH_EVENT, DrawPane::OnRefreshEvent)
END_EVENT_TABLE()

DrawPane::DrawPane(wxFrame * parent, TermWindow * termWindow) : wxPanel(parent)
        , m_RefreshNow(0)
        , m_RefreshLock()
        , m_TermWindow(termWindow)
        , m_Font(nullptr)
        , m_RefreshTimer(this, TIMER_ID)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InitColorTable();

    Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
}

DrawPane::~DrawPane()
{
    Disconnect( wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
}

void DrawPane::RequestRefresh()
{
    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        m_RefreshNow++;
    }

    wxCommandEvent event(MY_REFRESH_EVENT);

    // Do send it
    wxPostEvent(this, event);
}

void DrawPane::OnEraseBackground(wxEraseEvent & /*event*/)
{
}

void DrawPane::OnPaint(wxPaintEvent & /*event*/)
{
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    int refreshNow = 0;

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        refreshNow = m_RefreshNow;
    }

    {
        __ScopeLocker locker(buffer);
        wxAutoBufferedPaintDC dc(this);

        TermCellPtr cell = buffer->GetCurCell();
        cell->AddMode(TermCell::Cursor);
        DoPaint(dc, buffer, true);
        cell->RemoveMode(TermCell::Cursor);
    }

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        if (refreshNow)
            std::cout << "on paint end refresh:" << m_RefreshNow << "," << refreshNow << std::endl;
        m_RefreshNow -= refreshNow;
    }
}

void DrawPane::OnSize(wxSizeEvent& /*event*/)
{
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    wxSize clientSize = GetClientSize();
    wxClientDC dc(this);

    wxDCFontChanger fontChanger(dc, *GetFont());

    dc.GetTextExtent(SINGLE_WIDTH_CHARACTERS,
                     &m_CellWidth,
                     &m_LineHeight,
                     NULL,
                     NULL,
                     GetFont());

    printf("cell width:%u, line height:%u, %lu\n", dc.GetTextExtent(SINGLE_WIDTH_CHARACTERS).GetWidth(), m_LineHeight,
           sizeof(SINGLE_WIDTH_CHARACTERS));

    m_CellWidth /= (sizeof(SINGLE_WIDTH_CHARACTERS) - 1);

    printf("cell width:%u, line height:%u, %lu\n", m_CellWidth, m_LineHeight,
           sizeof(SINGLE_WIDTH_CHARACTERS));

    buffer->Resize((clientSize.GetHeight() - PADDING * 2) / m_LineHeight,
                   (clientSize.GetWidth() - PADDING * 2) / m_CellWidth);

    TermNetworkPtr network = context->GetTermNetwork();

    if (network)
        network->Resize(buffer->GetRows(),
                        buffer->GetCols());

    RequestRefresh();
}

wxFont * DrawPane::GetFont()
{
    if (m_Font)
        return m_Font;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return nullptr;

    AppConfigPtr appConfig = context->GetAppConfig();

    pybind11::gil_scoped_acquire acquire;
    auto font_size = appConfig->GetEntryUInt64("/term/font/size", 16);
    auto font_name = appConfig->GetEntry("/term/font/name", "Monospace");

    m_Font = new wxFont(wxFontInfo(font_size)
                        .FaceName(font_name.c_str())
                        .Family(wxFONTFAMILY_TELETYPE)
                        .Encoding(wxFONTENCODING_UTF8));

    return m_Font;
}

void DrawPane::OnIdle(wxIdleEvent& evt)
{
    PaintOnDemand();
    evt.RequestMore(false); // render continuously, not only once on idle
}

void DrawPane::InitColorTable()
{
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());
    TermColorThemePtr color_theme = context->GetTermColorTheme();

   try
    {
        pybind11::gil_scoped_acquire acquire;

        for(int i = 0; i < TermCell::ColorIndexCount;i++)
        {
            TermColorPtr color = color_theme->GetColor(i);

            m_ColorTable[i].Set(color->r,
                                color->g,
                                color->b,
                                color->a);
        }
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error Send:"
                  << std::endl
                  << e.what()
                  << std::endl;
        PyErr_Print();
    }
    catch(...)
    {
        std::cerr << "!!Error Send"
                  << std::endl;
        PyErr_Print();
    }
}

void DrawPane::OnTimer(wxTimerEvent& event)
{
    (void)event;
    PaintOnDemand();
}

void DrawPane::OnRefreshEvent(wxCommandEvent& event)
{
    (void)event;
    PaintOnDemand();

    if (m_RefreshNow) {
        wxCommandEvent event(MY_REFRESH_EVENT);

        wxPostEvent(this, event);
    }
}
