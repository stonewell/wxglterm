#include <pybind11/embed.h>

#include <iostream>

#include "term_window.h"
#include "draw_pane.h"

#include <wx/dcbuffer.h>
#include <wx/clipbrd.h>

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
        , m_Buffer{}
        , m_AppDebug{false}
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InitColorTable();

    m_AppDebug = m_TermWindow->GetPluginContext()->GetAppConfig()->GetEntryBool("app_debug", false);
    Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
}

DrawPane::~DrawPane()
{
    Disconnect( wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
    m_Buffer = nullptr;
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
    //wxWakeUpIdle();

    // if (!buffer)
    // {
    // TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    // if (!context)
    //     return;

    // buffer = context->GetTermBuffer();
    // }
    // if (!buffer)
    //     return;

    // wxRect clientSize = GetClientSize();

    // wxRegion clipRegion(clientSize);

    // CalculateClipRegion(clipRegion, buffer);

    // wxRegionIterator upd(clipRegion);
    // while (upd)
    // {
    //     RefreshRect(upd.GetRect());
    //     upd++;
    // }

    //m_RefreshTimer.StartOnce(20);
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

    {
        __ScopeLocker locker(buffer);
        wxAutoBufferedPaintDC dc(this);

        TermCellPtr cell = buffer->GetCurCell();
        if (cell)
            cell->AddMode(TermCell::Cursor);
        else {
            TermLinePtr line = buffer->GetCurLine();

            if (line)
                line->SetModified(true);
        }

        wxRegion upd(GetUpdateRegion()); // get the update rect list

        auto rows = buffer->GetRows();

        std::vector<uint32_t> updateRows;
        wxRect clientSize = GetClientSize();

        for (auto row = 0u; row < rows; row++) {
            auto line = buffer->GetLine(row);

            wxRect rowRect(0, PADDING + row * m_LineHeight, clientSize.GetWidth(), m_LineHeight);

            if (wxOutRegion != upd.Contains(rowRect)) {
                updateRows.push_back(row);
            }
        }

        DoPaint(dc, buffer, true, updateRows);

        if (cell)
            cell->RemoveMode(TermCell::Cursor);
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

    if (m_AppDebug)
        printf("cell width:%u, line height:%u, %lu\n", dc.GetTextExtent(SINGLE_WIDTH_CHARACTERS).GetWidth(), m_LineHeight,
               sizeof(SINGLE_WIDTH_CHARACTERS));

    m_CellWidth /= (sizeof(SINGLE_WIDTH_CHARACTERS) - 1);

    if (m_AppDebug)
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
    (void)evt;
    PaintOnDemand();

    //if (m_RefreshNow)
        //evt.RequestMore(); // render continuously, not only once on idle
        //wxWakeUpIdle();
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
        std::cerr << "!!Error InitColorTable:"
                  << std::endl
                  << e.what()
                  << std::endl;
        PyErr_Print();
    }
    catch(...)
    {
        std::cerr << "!!Error InitColorTable"
                  << std::endl;
        PyErr_Print();
    }
}

void DrawPane::OnTimer(wxTimerEvent& event)
{
    (void)event;
    PaintOnDemand();
    //Refresh();
}

void DrawPane::OnRefreshEvent(wxCommandEvent& event)
{
    (void)event;
    // PaintOnDemand();

    // if (m_RefreshNow) {
    //     wxCommandEvent event(MY_REFRESH_EVENT);

    //     wxPostEvent(this, event);
    // }
    m_RefreshTimer.StartOnce(20);
}

TermBufferPtr DrawPane::EnsureTermBuffer()
{
    if (!m_Buffer)
    {
        TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

        if (!context)
            return TermBufferPtr {};

        m_Buffer = context->GetTermBuffer();
    }

    return m_Buffer;
}
