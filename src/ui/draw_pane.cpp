#include <pybind11/embed.h>

#include "term_window.h"
#include "draw_pane.h"
#include <wx/dcbuffer.h>

#include "term_context.h"
#include "term_buffer.h"
#include "term_line.h"
#include "term_cell.h"

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
END_EVENT_TABLE()

DrawPane::DrawPane(wxFrame * parent, TermWindow * termWindow)
: wxPanel(parent)
        , m_RefreshNow(0)
        , m_RefreshLock()
        , m_BufferResizeLock()
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

    wxCriticalSectionLocker locker(m_BufferResizeLock);
    buffer->Resize((clientSize.GetHeight() - PADDING * 2) / m_LineHeight,
                   (clientSize.GetWidth() - PADDING * 2) / m_CellWidth);
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

    m_Font = new wxFont(wxFontInfo(font_size).FaceName(font_name));

    return m_Font;
}

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

    wxString content {""};

    {
        wxCriticalSectionLocker locker(m_BufferResizeLock);
        auto rows = buffer->GetRows();
        auto cols = buffer->GetCols();

        for (auto row = 0u; row < rows; row++) {
            auto line = buffer->GetLine(row);

            for (auto col = 0u; col < cols; col++) {
                auto cell = line->GetCell(col);

                if (cell && cell->GetChar() != 0) {
                    content.append(cell->GetChar());
                } else if (!cell) {
                    content.append(wxT(' '));
                }
            }

            content.append(wxT('\n'));
        }
    }

    dc.DrawText(content, PADDING, PADDING);
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
