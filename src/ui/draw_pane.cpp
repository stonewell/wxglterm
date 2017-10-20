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

constexpr uint32_t PADDING = 5;

#define SINGLE_WIDTH_CHARACTERS \
					" !\"#$%&'()*+,-./" \
					"0123456789" \
					":;<=>?@" \
					"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
					"[\\]^_`" \
					"abcdefghijklmnopqrstuvwxyz" \
					"{|}~"

BEGIN_EVENT_TABLE(DrawPane, wxPanel)
        EVT_PAINT(DrawPane::OnPaint)
        EVT_SIZE(DrawPane::OnSize)
EVT_KEY_DOWN(DrawPane::OnKeyDown)
EVT_KEY_UP(DrawPane::OnKeyUp)
EVT_CHAR(DrawPane::OnChar)
END_EVENT_TABLE()

DrawPane::DrawPane(wxFrame * parent, TermWindow * termWindow)
: wxPanel(parent)
        , m_RefreshNow(0)
        , m_RefreshLock()
        , m_TermWindow(termWindow)
        , m_Font(nullptr)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Connect( wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
}

DrawPane::~DrawPane()
{
    Disconnect( wxEVT_IDLE, wxIdleEventHandler(DrawPane::OnIdle) );
}

void DrawPane::RequestRefresh()
{
    wxCriticalSectionLocker locker(m_RefreshLock);
    m_RefreshNow++;
}

void DrawPane::OnPaint(wxPaintEvent & /*event*/)
{
    wxAutoBufferedPaintDC dc(this);

    DoPaint(dc);
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

    dc.GetTextExtent(SINGLE_WIDTH_CHARACTERS,
                     &m_CellWidth,
                     &m_LineHeight,
                     NULL,
                     NULL,
                     GetFont());

    m_CellWidth /= (sizeof(SINGLE_WIDTH_CHARACTERS) - 1);
    m_LineHeight += 1;

    printf("cell width:%u, line height:%u, %lu\n", m_CellWidth, m_LineHeight,
           sizeof(SINGLE_WIDTH_CHARACTERS));

    buffer->Resize((clientSize.GetHeight() - PADDING * 2) / m_LineHeight,
                   (clientSize.GetWidth() - PADDING * 2) / m_CellWidth);

    TermNetworkPtr network = context->GetTermNetwork();

    if (network)
        network->Resize(buffer->GetRows(),
                        buffer->GetCols());
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

    m_Font = new wxFont(wxFontInfo(font_size).FaceName(font_name.c_str()));

    return m_Font;
}

class __ScopeLocker {
public:
    __ScopeLocker(TermBufferPtr termBuffer) :
        m_TermBuffer(termBuffer)
    {
        m_TermBuffer->LockResize();
    }

    ~__ScopeLocker() {
        m_TermBuffer->UnlockResize();
    }

    TermBufferPtr m_TermBuffer;
};

void DrawPane::DoPaint(wxDC & dc)
{
    dc.SetBackground( *wxWHITE_BRUSH );
    dc.Clear();

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    wxDCFontChanger fontChanger(dc, *GetFont());

    {
        __ScopeLocker locker(buffer);

        auto rows = buffer->GetRows();
        auto cols = buffer->GetCols();

        auto y = PADDING;

        for (auto row = 0u; row < rows; row++) {
            auto line = buffer->GetLine(row);

            wxString content {""};

            for (auto col = 0u; col < cols; col++) {
                auto cell = line->GetCell(col);

                if (cell && cell->GetChar() != 0) {
                    content.append(cell->GetChar());
                } else if (!cell) {
                    content.append(wxT(' '));
                }
            }

            dc.DrawText(content, PADDING, y);
            y += m_LineHeight;
        }
    }

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
        wxBufferedDC bDC(&dc,
                         GetClientSize());
        DoPaint(bDC);
    }

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        m_RefreshNow -= refreshNow;
    }

    evt.RequestMore(); // render continuously, not only once on idle
}

void DrawPane::OnKeyDown(wxKeyEvent& event)
{
    (void)event;
    event.Skip();
}

void DrawPane::OnKeyUp(wxKeyEvent& event)
{
    (void)event;
}

void DrawPane::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();

    char c= uc & 0xFF;
    if (uc == WXK_NONE)
    {
        printf("key code:%d\n", event.GetKeyCode());

        return ;
    }

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();

   try
    {
        pybind11::gil_scoped_acquire acquire;
        char ch[2] = {c, 0};
        network->Send(ch, 1);
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
