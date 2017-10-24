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

extern
wxCoord PADDING;

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

void DrawPane::DrawContent(wxDC &dc,
                           wxString & content,
                           uint16_t & last_fore_color,
                           uint16_t & last_back_color,
                           uint16_t & last_mode,
                           uint16_t fore_color,
                           uint16_t back_color,
                           uint16_t mode,
                           wxCoord & last_x,
                           wxCoord & last_y,
                           wxCoord y)
{
    (void)mode;
    (void)last_mode;

    wxSize content_size = dc.GetTextExtent(content);
    bool multi_line = content.Find('\n', true) > 0;

    wxSize content_last_line_size {0, 0};
    wxSize content_before_last_line_size {0, 0};

    if (multi_line)
    {
        content_last_line_size = dc.GetTextExtent(content.AfterLast('\n'));
        content_before_last_line_size.SetHeight(content_size.GetHeight() - content_last_line_size.GetHeight());
        content_before_last_line_size.SetWidth(content_size.GetWidth());
    }

    if (last_back_color != TermCell::DefaultBackColorIndex)
    {
        wxBrush brush(m_ColorTable[last_back_color]);
        dc.SetBrush(brush);
        dc.SetPen(wxPen(m_ColorTable[last_back_color]));

        if (multi_line)
        {
            dc.DrawRectangle(wxPoint(last_x, last_y),
                             content_before_last_line_size);
            dc.DrawRectangle(wxPoint(PADDING, last_y +
                                     content_before_last_line_size.GetHeight()),
                             content_last_line_size);
        }
        else
        {
            dc.DrawRectangle(wxPoint(last_x, last_y), content_size);
        }
    }

    dc.SetTextForeground(m_ColorTable[last_fore_color]);
    dc.SetTextBackground(m_ColorTable[last_back_color]);
    dc.DrawText(content, last_x, last_y);

    if (multi_line)
    {
        last_x = PADDING + content_last_line_size.GetWidth();
    } else {
        last_x += content_size.GetWidth();
    }

    last_y = y;
    content.Clear();
    last_fore_color = fore_color;
    last_back_color = back_color;
}

void DrawPane::CalculateClipRegion(wxRegion & clipRegion)
{
   TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    wxRect clientSize = GetClientSize();

    clipRegion.Union(clientSize);

    auto rows = buffer->GetRows();

    for (auto row = 0u; row < rows; row++) {
        auto line = buffer->GetLine(row);

        if (!clipRegion.IsEmpty()
            && row == line->GetLastRenderLineIndex()
            && !line->IsModified())
        {
            wxRect rowRect(0, PADDING + row * m_LineHeight, clientSize.GetWidth(), m_LineHeight);
            clipRegion.Subtract(rowRect);
        }
    }
}

void DrawPane::DoPaint(wxDC & dc, bool full_paint)
{
    __DCAttributesChanger changer(&dc);

    wxBrush backgroundBrush(m_ColorTable[TermCell::DefaultBackColorIndex]);

    wxString content {""};

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    wxDCFontChanger fontChanger(dc, *GetFont());

    auto rows = buffer->GetRows();
    auto cols = buffer->GetCols();

    auto y = PADDING;

    uint16_t last_fore_color = TermCell::DefaultForeColorIndex;
    uint16_t last_back_color = TermCell::DefaultBackColorIndex;
    wxCoord last_y = PADDING;
    wxCoord last_x = PADDING;
    uint16_t last_mode = 0;

    dc.SetBackground( backgroundBrush );
    dc.Clear();

    for (auto row = 0u; row < rows; row++) {
        auto line = buffer->GetLine(row);

        if (!full_paint &&
            row == line->GetLastRenderLineIndex()
            && !line->IsModified())
        {
            y += m_LineHeight;

            if (content.Length() > 0)
            {
                DrawContent(dc, content,
                            last_fore_color,
                            last_back_color,
                            last_mode,
                            TermCell::DefaultForeColorIndex,
                            TermCell::DefaultBackColorIndex,
                            0,
                            last_x,
                            last_y,
                            y);
            }

            last_x = PADDING;
            last_y = y;

            continue;
        }

        line->SetLastRenderLineIndex(row);
        line->SetModified(false);

        for (auto col = 0u; col < cols; col++) {
            auto cell = line->GetCell(col);

            wchar_t ch = 0;
            uint16_t fore_color = TermCell::DefaultForeColorIndex;
            uint16_t back_color = TermCell::DefaultBackColorIndex;
            uint16_t mode = 0;

            if (cell && cell->GetChar() != 0) {
                fore_color = cell->GetForeColorIndex();
                back_color = cell->GetBackColorIndex();
                mode = cell->GetMode();
                ch = cell->GetChar();
            } else if (!cell) {
                ch = ' ';
            }

            if (ch != 0)
            {
                if (last_fore_color != fore_color
                    || last_back_color != back_color)
                {
                    DrawContent(dc, content,
                                last_fore_color,
                                last_back_color,
                                last_mode,
                                fore_color,
                                back_color,
                                mode,
                                last_x,
                                last_y,
                                y);

                }

                content.append(ch);
            }

        }

        y += m_LineHeight;

        if (last_x == PADDING)
        {
            if (row != rows - 1)
                content.append("\n");
        }
        else if (content.Length() > 0)
        {
            DrawContent(dc, content,
                        last_fore_color,
                        last_back_color,
                        last_mode,
                        TermCell::DefaultForeColorIndex,
                        TermCell::DefaultBackColorIndex,
                        0,
                        last_x,
                        last_y,
                        y);
            last_x = PADDING;
            last_y = y;
        }
    }

    if (content.Length() > 0)
    {
        dc.SetTextForeground(m_ColorTable[last_fore_color]);
        dc.SetTextBackground(m_ColorTable[last_back_color]);

        dc.DrawText(content, last_x, last_y);
    }
}
