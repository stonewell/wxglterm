#include <pybind11/embed.h>

#include <iostream>

#include "term_window.h"
#include "draw_pane.h"

#include <wx/dcbuffer.h>
#include <wx/clipbrd.h>
#include <wx/region.h>

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

#define USE_IDLE_EVENT 1
#define USE_REQUEST_FRESH 0

#define FPS (120)
constexpr wxDouble REFRESH_DELTA = (1000.0 / FPS);

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
EVT_MOUSE_EVENTS(DrawPane::OnMouseEvent)
END_EVENT_TABLE()

DrawPane::DrawPane(wxFrame * parent, TermWindow * termWindow) : wxWindow(parent, wxID_ANY)
        , m_RefreshNow(0)
        , m_RefreshLock()
        , m_TermWindow(termWindow)
        , m_Fonts{nullptr}
        , m_RefreshTimer(this, TIMER_ID)
        , m_Buffer{}
        , m_AppDebug{false}
        , m_EnableMouseTrack{false}
        , m_SavedMouseButton(-1)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    InitColorTable();

    m_AppDebug = m_TermWindow->GetPluginContext()->GetAppConfig()->GetEntryBool("app_debug", false);
    m_LastPaintTime = wxGetLocalTimeMillis();
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

#if USE_REQUEST_FRESH
    wxCommandEvent event(MY_REFRESH_EVENT);

    // Do send it
    wxPostEvent(this, event);
#endif
}

void DrawPane::OnEraseBackground(wxEraseEvent & /*event*/)
{
}

void DrawPane::OnPaint(wxPaintEvent & /*event*/)
{
    TermBufferPtr buffer = std::move(EnsureTermBuffer());

    if (!buffer)
        return;

    {
        __ScopeLocker locker(buffer);
        wxAutoBufferedPaintDC dc(this);

        TermCellPtr cell = buffer->GetCurCell();
        if (cell) {
            if (cell->GetChar() == 0 && buffer->GetCol() > 0) {
                cell = buffer->GetCell(buffer->GetRow(), buffer->GetCol() - 1);
            }

            if (cell) {
                cell->AddMode(TermCell::Cursor);
            }
        } else {
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

    m_LastPaintTime = wxGetLocalTimeMillis();
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
        std::cout << "cell width:"
                  << dc.GetTextExtent(SINGLE_WIDTH_CHARACTERS).GetWidth()
                  << ", line height:"
                  << m_LineHeight
                  << ", "
                  << sizeof(SINGLE_WIDTH_CHARACTERS)
                  << std::endl;

    m_CellWidth /= (sizeof(SINGLE_WIDTH_CHARACTERS) - 1);

    if (m_AppDebug)
        std::cout << "cell width:" << m_CellWidth
                  << ", line height:" << m_LineHeight
                  << ", " << sizeof(SINGLE_WIDTH_CHARACTERS)
                  << std::endl;

    buffer->Resize((clientSize.GetHeight() - PADDING * 2) / m_LineHeight,
                   (clientSize.GetWidth() - PADDING * 2) / m_CellWidth);

    TermNetworkPtr network = context->GetTermNetwork();

    if (network)
        network->Resize(buffer->GetRows(),
                        buffer->GetCols());

    RequestRefresh();
}

wxFont * DrawPane::GetFont(FontCategoryEnum font_category)
{
    if (font_category >= DrawPane::FontCategoryCount)
        font_category = DrawPane::Default;

    if (m_Fonts[font_category])
        return m_Fonts[font_category];

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return wxFont::New(16, wxFONTFAMILY_TELETYPE);

    AppConfigPtr appConfig = context->GetAppConfig();

    pybind11::gil_scoped_acquire acquire;
    auto font_size = appConfig->GetEntryUInt64("/term/font/size", 16);
    auto font_name = appConfig->GetEntry("/term/font/name", "");

    m_Fonts[font_category] = new wxFont(wxFontInfo(font_size)
                        .FaceName(font_name.c_str())
                        .Family(wxFONTFAMILY_TELETYPE)
                        .Encoding(wxFONTENCODING_UTF8));

    switch(font_category) {
    case Bold:
        m_Fonts[font_category]->MakeBold();
        break;
    case Underlined:
        m_Fonts[font_category]->MakeUnderlined();
        break;
    case BoldUnderlined:
        m_Fonts[font_category]->MakeBold();
        m_Fonts[font_category]->MakeUnderlined();
        break;
    default:
        break;
    };

    return m_Fonts[font_category];
}

void DrawPane::OnIdle(wxIdleEvent& evt)
{
    (void)evt;

#if USE_IDLE_EVENT
    wxLongLong now = wxGetLocalTimeMillis();

    if (now - REFRESH_DELTA >= m_LastPaintTime && m_RefreshNow) {
        PaintOnDemand();

        std::cout << "idle paint time:"
                  << now
                  << ","
                  << (wxGetLocalTimeMillis() - now)
                  << std::endl;

        m_LastPaintTime = wxGetLocalTimeMillis();
    }

    evt.RequestMore();
#endif
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
    wxLongLong now = wxGetLocalTimeMillis();
#if USE_REQUEST_FRESH
    PaintOnDemand();
#endif
    m_LastPaintTime = wxGetLocalTimeMillis();

    std::cout << "paint time:" << (m_LastPaintTime - now)
              << std::endl;
}

void DrawPane::OnRefreshEvent(wxCommandEvent& event)
{
    (void)event;
#if USE_REQUEST_FRESH
    m_RefreshTimer.StartOnce(REFRESH_DELTA);
#endif
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
