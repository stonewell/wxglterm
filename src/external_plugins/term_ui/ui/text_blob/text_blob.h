#pragma once

#include <wx/wx.h>
#include <unordered_map>

class wxTextBlob {
public:
    wxTextBlob();
    virtual ~wxTextBlob() = default;

public:
    virtual void Render(wxGraphicsContext * context);
    virtual wxPoint AddText(const wxString & text, const wxPoint pt, const wxFont * pFont, wxColour fore_color, wxColour back_color);
    //if not set, it will use the default glyph advancex, otherwise all glyph use the set value, wide char will use double size
    void SetGlyphAdvanceX(wxCoord advance) { m_GlyphAdvanceX = advance; }
    wxCoord GetGlyphAdvanceX() const { return m_GlyphAdvanceX; }
    void SetLineHeight(wxCoord h) { m_LineHeight = h; }
    wxCoord GetLineHeight() const { return m_LineHeight; }
    void SetPPI(wxSize ppi) { m_PPI = ppi; }
    wxSize GetPPI() const { return m_PPI; }

    typedef struct __TextPart {
        wxString text;
        wxPoint pt;
        const wxFont * pFont;
        wxColour fore;
        wxColour back;
        wxSize size;
    } TextPart;

    typedef struct __BackgroundRectAttrs {
        wxRect rect;
        wxColour back;
    } BackgroundRectAttrs;

    using TextPartVector = wxVector<struct __TextPart>;
    using BackgroundRectVector = wxVector<struct __BackgroundRectAttrs>;

private:
    wxScopedPtr<wxGraphicsContext> m_TextExtentContext;
    wxCoord m_GlyphAdvanceX;
    wxCoord m_LineHeight;
    TextPartVector m_TextParts;
    wxSize m_PPI;

protected:
    virtual void * BeginTextRendering();
    virtual void EndTextRendering(void * rendering_data);
    virtual void DoDrawBackground(wxGraphicsContext * context, void * rendering_data);
    virtual void DoDrawText(wxGraphicsContext * context, void * rendering_data);
};
