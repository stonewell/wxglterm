#pragma once

#include <wx/wx.h>
#include "font_manager.h"

class wxTextBlob {
public:
    wxTextBlob();
    virtual ~wxTextBlob() = default;

public:
    void Render(wxGraphicsContext * context);
    wxPoint AddText(const wxString & text, const wxPoint pt, const wxFont * pFont, wxColour fore_color, wxColour back_color);
    //if not set, it will use the default glyph advancex, otherwise all glyph use the set value, wide char will use double size
    void SetGlyphAdvanceX(wxCoord advance) { m_GlyphAdvanceX = advance; }
    wxCoord GetGlyphAdvanceX() const { return m_GlyphAdvanceX; }
    void SetLineHeight(wxCoord h) { m_LineHeight = h; }
    wxCoord GetLineHeight() const { return m_LineHeight; }

private:
    typedef struct __TextPart {
        wxString text;
        wxPoint pt;
        const wxFont * pFont;
        wxColour fore;
        wxColour back;
        wxSize size;
    } TextPart;

    using TextPartVector = wxVector<struct __TextPart>;

    wxScopedPtr<wxGraphicsContext> m_TextExtentContext;
    wxCoord m_GlyphAdvanceX;
    wxCoord m_LineHeight;
    TextPartVector m_TextParts;
    fttb::FontManagerPtr m_FontManager;

    void DoDrawText(wxGraphicsContext * context, const TextPart & text_part);

    wxString wxFontToFCDesc(const wxFont * pFont);
};