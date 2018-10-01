#include <pybind11/embed.h>

#include <iostream>

#include "term_window.h"
#include "draw_pane.h"

#include <wx/dcbuffer.h>
#include "wx/dcgraph.h"
#include "wx/graphics.h"

#include "term_context.h"
#include "term_buffer.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_network.h"
#include "color_theme.h"
#include "scope_locker.h"
#include "char_width.h"

#include <bitset>

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
                           std::bitset<16> & buffer_mode,
                           uint32_t & last_fore_color,
                           uint32_t & last_back_color,
                           uint32_t & last_mode,
                           uint32_t fore_color,
                           uint32_t back_color,
                           uint32_t mode,
                           wxCoord & last_x,
                           wxCoord & last_y,
#if USE_TEXT_BLOB
                           wxTextBlob & text_blob
#else
                           bool drawBySingleChar
#endif
                           )
{
    wxSize content_size = dc.GetMultiLineTextExtent(content);
    bool multi_line = content.Find('\n', true) > 0;

    wxSize content_last_line_size {0, 0};
    wxSize content_before_last_line_size {0, 0};

    std::bitset<16> m(last_mode);

    if (multi_line)
    {
        content_last_line_size = dc.GetTextExtent(content.AfterLast('\n'));
        content_before_last_line_size.SetHeight(content_size.GetHeight() - content_last_line_size.GetHeight());
        content_before_last_line_size.SetWidth(content_size.GetWidth());
    }

    uint32_t back_color_use = last_back_color;
    uint32_t fore_color_use = last_fore_color;

    wxFont * font(GetFont());

    if (m.test(TermCell::Bold) ||
        buffer_mode.test(TermCell::Bold)) {
        if (back_color_use < 8)
            back_color_use += 8;

        if (fore_color_use < 8)
            fore_color_use += 8;

        font = GetFont(DrawPane::Bold);
    }

#if !USE_TEXT_BLOB
    dc.SetFont(*font);
#endif

    if (m.test(TermCell::Cursor))
    {
        back_color_use = TermCell::DefaultCursorColorIndex;
        fore_color_use = last_back_color;
    }

    if (m.test(TermCell::Reverse) ||
        buffer_mode.test(TermCell::Reverse))
    {
        uint32_t tmp = back_color_use;
        back_color_use = fore_color_use;
        fore_color_use = tmp;
    }

#if !USE_TEXT_BLOB
    if (back_color_use != TermCell::DefaultBackColorIndex)
    {
        wxBrush brush(__GetColorByIndex(back_color_use));
        dc.SetBrush(brush);
        dc.SetPen(wxPen(__GetColorByIndex(back_color_use)));

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

    dc.SetTextForeground(__GetColorByIndex(fore_color_use));
    dc.SetTextBackground(__GetColorByIndex(back_color_use));

    if (!drawBySingleChar) {
        dc.DrawText(content, last_x, last_y);

        if (multi_line)
        {
            last_x = PADDING + content_last_line_size.GetWidth();
        } else {
            last_x += content_size.GetWidth();
        }

        last_y += content_size.GetHeight();
    } else {
        for(const auto & c : content) {
            if (c == '\n') {
                last_y += m_LineHeight;
                last_x = PADDING;
                continue;
            }

            auto w = char_width(c);

            if (!w || w == (size_t)-1) continue;

            dc.DrawText(c, last_x, last_y);

            last_x += w * m_CellWidth;
        }
    }
#else
    text_blob.AddText(content, {last_x, last_y},
                      font,
                      __GetColorByIndex(fore_color_use),
                      __GetColorByIndex(back_color_use));

    if (multi_line)
    {
        last_x = PADDING + content_last_line_size.GetWidth();
    } else {
        last_x += content_size.GetWidth();
    }

    last_y += content_size.GetHeight();
#endif

    content.Clear();
    last_fore_color = fore_color;
    last_back_color = back_color;
    last_mode = mode;
}

void DrawPane::CalculateClipRegion(wxRegion & clipRegion, TermBufferPtr buffer)
{
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

static
bool contains(const std::vector<uint32_t> & rowsToDraw, uint32_t row) {
    for(auto & it : rowsToDraw) {
        if (it == row)
            return true;
    }

    return false;
}

void DrawPane::DoPaint(wxDC & dc, TermBufferPtr buffer, bool full_paint, const std::vector<uint32_t> & rowsToDraw)
{
    __DCAttributesChanger changer(&dc);

    wxBrush backgroundBrush(__GetColorByIndex(TermCell::DefaultBackColorIndex));

    wxString content {""};

    wxDCFontChanger fontChanger(dc, *GetFont());

    auto rows = buffer->GetRows();
    auto cols = buffer->GetCols();

    auto y = PADDING;

    uint32_t last_fore_color = TermCell::DefaultForeColorIndex;
    uint32_t last_back_color = TermCell::DefaultBackColorIndex;
    wxCoord last_y = PADDING;
    wxCoord last_x = PADDING;
    uint32_t last_mode = 0;

    dc.SetBackground( backgroundBrush );
    dc.Clear();
    std::bitset<16> m(buffer->GetMode());

#if USE_TEXT_BLOB
    wxTextBlob text_blob;
#endif

    for (auto row = 0u; row < rows; row++) {
        auto line = buffer->GetLine(row);

        if ((!full_paint &&
             row == line->GetLastRenderLineIndex()
             && !line->IsModified())
            || (rowsToDraw.size() > 0 && !contains(rowsToDraw, row)))
        {
            y += m_LineHeight;

            if (content.Length() > 0)
            {
                DrawContent(dc, content,
                            m,
                            last_fore_color,
                            last_back_color,
                            last_mode,
                            TermCell::DefaultForeColorIndex,
                            TermCell::DefaultBackColorIndex,
                            0,
                            last_x,
                            last_y
#if USE_TEXT_BLOB
                            ,text_blob
#endif
                            );
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
            uint32_t fore_color = TermCell::DefaultForeColorIndex;
            uint32_t back_color = TermCell::DefaultBackColorIndex;
            uint32_t mode = 0;

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
                    || last_back_color != back_color
                    || last_mode != mode)
                {
                    DrawContent(dc, content,
                                m,
                                last_fore_color,
                                last_back_color,
                                last_mode,
                                fore_color,
                                back_color,
                                mode,
                                last_x,
                                last_y
#if USE_TEXT_BLOB
                                ,text_blob
#endif
                                );
                    last_y = y;
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
                        m,
                        last_fore_color,
                        last_back_color,
                        last_mode,
                        TermCell::DefaultForeColorIndex,
                        TermCell::DefaultBackColorIndex,
                        0,
                        last_x,
                        last_y
#if USE_TEXT_BLOB
                        ,text_blob
#endif
                        );

            last_x = PADDING;
            last_y = y;
        }
    }

    if (content.Length() > 0)
    {
        DrawContent(dc, content,
                    m,
                    last_fore_color,
                    last_back_color,
                    last_mode,
                    TermCell::DefaultForeColorIndex,
                    TermCell::DefaultBackColorIndex,
                    0,
                    last_x,
                    last_y
#if USE_TEXT_BLOB
                    ,text_blob
#endif
                    );
    }

#if USE_TEXT_BLOB
    if ( wxGCDC *gcdc = wxDynamicCast(&dc, wxGCDC) ) {
        text_blob.Render(gcdc->GetGraphicsContext());
    }
#endif
}

void DrawPane::PaintOnDemand()
{
    (void)EnsureTermBuffer();

    if (!m_Buffer)
        return;

    int refreshNow = 0;

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        refreshNow = m_RefreshNow;
        if (refreshNow && m_AppDebug)
            std::cout << "refresh:" << refreshNow << std::endl;
    }

    if (refreshNow)
    {
#if USE_TEXT_BLOB
        wxGCDC dc;
        wxPaintDC paintDC(this);
        dc.SetGraphicsContext(wxGraphicsContext::Create(paintDC));
#else
        wxClientDC dc(this);
#endif

        wxRect clientSize = GetClientSize();

        wxRegion clipRegion(clientSize);

#if !USE_TEXT_BLOB
        wxBufferedDC bDC(&dc,
                         GetClientSize());
#endif
        __ScopeLocker buffer_locker(m_Buffer);
        if (m_AppDebug)
            std::cout << "buffer locked to draw" << std::endl;

        bool paintChanged = true;

        TermCellPtr cell = m_Buffer->GetCurCell();

        if (cell) {
            if (cell->GetChar() == 0 && m_Buffer->GetCol() > 0) {
                cell = m_Buffer->GetCell(m_Buffer->GetRow(), m_Buffer->GetCol() - 1);
            }

            if (cell) {
                cell->AddMode(TermCell::Cursor);
            }
        }
        else {
            TermLinePtr line = m_Buffer->GetCurLine();

            if (line)
                line->SetModified(true);
        }

        if (paintChanged)
        {
            CalculateClipRegion(clipRegion, m_Buffer);

            dc.DestroyClippingRegion();
            dc.SetDeviceClippingRegion(clipRegion);
        }

#if USE_TEXT_BLOB
        DoPaint(dc, m_Buffer, !paintChanged);
#else
        DoPaint(bDC, m_Buffer, !paintChanged);
#endif
        if (cell)
            cell->RemoveMode(TermCell::Cursor);
        if (m_AppDebug)
            std::cout << "buffer draw done, unlock" << std::endl;
    }

    {
        wxCriticalSectionLocker locker(m_RefreshLock);
        if (refreshNow && m_AppDebug)
            std::cout << "end refresh:" << m_RefreshNow << "," << refreshNow << std::endl;
        m_RefreshNow -= refreshNow;
    }
}

wxColour DrawPane::__GetColorByIndex(uint32_t index) {
    if (index >= TermCell::ColorIndexCount) {
        index -= TermCell::ColorIndexCount;

        wxColour c;
        c.Set((index >> 16) & 0xFF,
              (index >> 8) & 0xFF,
              (index) & 0xFF,
              0xFF);

        return c;
    }

    return m_ColorTable[index];
}
