#pragma once

#include <wx/wx.h>
#include "term_cell.h"
#include <bitset>
#include <vector>

#include "text_blob.h"

#define USE_TEXT_BLOB 1

class DrawPane : public wxPanel {
public:
    DrawPane(wxFrame * parent,
             TermWindow * termWindow);
    virtual ~DrawPane();

    DECLARE_EVENT_TABLE();

public:
    enum FontCategoryEnum {
        Default = 0,
        Bold,
        Underlined,
        BoldUnderlined,

        FontCategoryCount
    };

    void RequestRefresh();

    uint32_t GetColorByIndex(uint32_t index) {
        const auto & c = __GetColorByIndex(index);

        uint32_t v = c.Red();
        v = (v << 8) | c.Green();
        v = (v << 8) | c.Blue();
        v = (v << 8) | c.Alpha();

        return v;
    }

    void SetColorByIndex(uint32_t index, uint32_t v) {
        if (index >= TermCell::ColorIndexCount)
            return;

        m_ColorTable[index].Set((v >> 24) & 0xFF,
                                (v >> 16) & 0xFF,
                                (v >> 8) & 0xFF,
                                v & 0xFF);
    }

    void EnableMouseTrack(bool enable) {
        m_EnableMouseTrack = enable;
    }

    uint32_t GetWidth() {
        wxRect clientSize = GetClientSize();
        return clientSize.GetWidth();
    }

    uint32_t GetHeight() {
        wxRect clientSize = GetClientSize();
        return clientSize.GetHeight();
    }

    uint32_t GetLineHeight() {
        return m_LineHeight;
    }

    uint32_t GetColWidth() {
        return m_CellWidth;
    }
private:
    void OnPaint(wxPaintEvent &evt);
    void OnIdle(wxIdleEvent& evt);
    void OnSize(wxSizeEvent& event);
    void OnKeyDown(wxKeyEvent& event);
    void OnKeyUp(wxKeyEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnEraseBackground(wxEraseEvent & event);
    void OnTimer(wxTimerEvent& event);
    void OnRefreshEvent(wxCommandEvent& event);
    void OnMouseEvent(wxMouseEvent& event);

    wxFont * GetFont(FontCategoryEnum font_category = DrawPane::Default);

    void PaintOnDemand();
    void DoPaint(wxDC & dc
                 , TermBufferPtr buffer
                 , bool full_paint
                 , const std::vector<uint32_t> & rowsToDraw = std::vector<uint32_t>());
    void CalculateClipRegion(wxRegion & clipRegion, TermBufferPtr buffer);

    void DrawContent(wxDC &dc,
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
                     bool drawBySingleChar = false
#endif
                     );

    void InitColorTable();

    wxColour __GetColorByIndex(uint32_t index);

    TermBufferPtr EnsureTermBuffer();
    void GetMousePos(wxMouseEvent & event, uint32_t & row, uint32_t & col);

    int m_RefreshNow;
    wxCriticalSection m_RefreshLock;
    TermWindow * m_TermWindow;

    wxFont * m_Fonts[DrawPane::FontCategoryCount];
    wxCoord m_LineHeight;
    wxCoord m_CellWidth;

    wxColour m_ColorTable[TermCell::ColorIndexCount];
    wxTimer m_RefreshTimer;
    TermBufferPtr m_Buffer;

    bool m_AppDebug;

    bool m_EnableMouseTrack;
    int m_SavedMouseButton;

    wxRegion m_UpdateRegion;
};
