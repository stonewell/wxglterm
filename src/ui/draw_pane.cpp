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
    InitColorTable();

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

class __DCAttributesChanger {
public:
    __DCAttributesChanger(wxDC * pdc) :
        m_pDC(pdc)
        , back(pdc->GetBackground())
        , txt_fore(pdc->GetTextForeground())
        , txt_back(pdc->GetTextBackground())
    {
    }

    ~__DCAttributesChanger() {
        m_pDC->SetTextForeground(txt_fore);
        m_pDC->SetTextBackground(txt_back);
        m_pDC->SetBackground(back);
    }

    wxDC * m_pDC;
    const wxBrush & back;
    const wxColour & txt_fore;
    const wxColour & txt_back;
};

void DrawPane::DoPaint(wxDC & dc)
{
    __DCAttributesChanger changer(&dc);

    wxBrush backgroundBrush(m_ColorTable[TermCell::DefaultBackColorIndex]);

    dc.SetBackground( backgroundBrush );
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

        uint16_t last_fore_color = TermCell::DefaultForeColorIndex;
        uint16_t last_back_color = TermCell::DefaultBackColorIndex;
        auto last_y = PADDING;
        auto last_x = PADDING;

        wxString content {""};

        for (auto row = 0u; row < rows; row++) {
            auto line = buffer->GetLine(row);

            auto x = PADDING;

            for (auto col = 0u; col < cols; col++) {
                auto cell = line->GetCell(col);

                wchar_t ch = 0;
                uint16_t fore_color = TermCell::DefaultForeColorIndex;
                uint16_t back_color = TermCell::DefaultBackColorIndex;

                if (cell && cell->GetChar() != 0) {
                    fore_color = cell->GetForeColorIndex();
                    back_color = cell->GetBackColorIndex();
                    ch = cell->GetChar();
                } else if (!cell) {
                    ch = ' ';
                }

                if (ch != 0)
                {
                    if (last_fore_color != fore_color
                        || last_back_color != back_color)
                    {
                        wxSize content_size = dc.GetTextExtent(content);

                        if (last_back_color != back_color)
                        {
                            wxBrush brush(m_ColorTable[last_back_color]);
                            dc.SetBrush(brush);
                            dc.SetPen(wxPen(m_ColorTable[last_back_color]));
                            dc.DrawRectangle(wxPoint(last_x, last_y), content_size);
                        }

                        dc.SetTextForeground(m_ColorTable[last_fore_color]);
                        dc.SetTextBackground(m_ColorTable[last_back_color]);
                        dc.DrawText(content, last_x, last_y);

                        if (content.Find('\n', true) > 0)
                        {
                            last_x = PADDING + dc.GetTextExtent(content.AfterLast('\n')).GetWidth();
                        } else {
                            last_x += content_size.GetWidth();
                        }
                        last_y = y;
                        content.Clear();
                        last_fore_color = fore_color;
                        last_back_color = back_color;

                    }

                    content.append(ch);
                }

                x += m_CellWidth;
            }

            y += m_LineHeight;

            if (last_x == PADDING)
            {
                content.append("\n");
            }
            else if (content.Length() > 0)
            {
                dc.SetTextForeground(m_ColorTable[last_fore_color]);
                dc.SetTextBackground(m_ColorTable[last_back_color]);
                dc.DrawText(content, last_x, last_y);
                last_x = PADDING;
                last_y = y;
                content.Clear();
                last_fore_color = TermCell::DefaultForeColorIndex;
                last_back_color = TermCell::DefaultBackColorIndex;
            }
        }

        if (content.Length() > 0)
        {
            dc.SetTextForeground(m_ColorTable[last_fore_color]);
            dc.SetTextBackground(m_ColorTable[last_back_color]);
            dc.DrawText(content, last_x, last_y);
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
