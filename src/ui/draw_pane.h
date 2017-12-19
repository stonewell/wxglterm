#pragma once

#include <wx/wx.h>
#include "term_cell.h"
#include <bitset>

class DrawPane : public wxPanel {
public:
    DrawPane(wxFrame * parent,
             TermWindow * termWindow);
    virtual ~DrawPane();

    DECLARE_EVENT_TABLE();

public:
    void RequestRefresh();

    uint32_t GetColorByIndex(uint32_t index) {
        return m_ColorTable[index].GetRGBA();
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

    wxFont * GetFont();

    void PaintOnDemand();
    void DoPaint(wxDC & dc
                 , TermBufferPtr buffer
                 , bool full_paint
                 , const std::vector<uint32_t> & rowsToDraw = std::vector<uint32_t>());
    void CalculateClipRegion(wxRegion & clipRegion, TermBufferPtr buffer);

    void DrawContent(wxDC &dc,
                     wxString & content,
                     std::bitset<16> & buffer_mode,
                     uint16_t & last_fore_color,
                     uint16_t & last_back_color,
                     uint16_t & last_mode,
                     uint16_t fore_color,
                     uint16_t back_color,
                     uint16_t mode,
                     wxCoord & last_x,
                     wxCoord & last_y,
                     wxCoord y);

    void InitColorTable();

    TermBufferPtr EnsureTermBuffer();

    int m_RefreshNow;
    wxCriticalSection m_RefreshLock;
    TermWindow * m_TermWindow;

    wxFont * m_Font;
    wxCoord m_LineHeight;
    wxCoord m_CellWidth;

    wxColour m_ColorTable[TermCell::ColorIndexCount];
    wxTimer m_RefreshTimer;
    TermBufferPtr m_Buffer;

    bool m_AppDebug;
};
