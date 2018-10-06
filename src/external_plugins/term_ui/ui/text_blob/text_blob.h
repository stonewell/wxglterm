#pragma once

#include <wx/wx.h>
#include <unordered_map>

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

    typedef struct __GlyphAttrs {
        wchar_t codepoint;
        wxPoint pt;
    } GlyphAttrs;

    typedef struct __BackgroundRectAttrs {
        wxRect rect;
        wxColour back;
    } BackgroundRectAttrs;

    using TextPartVector = wxVector<struct __TextPart>;
    using CodepointVector = std::vector<GlyphAttrs>;
    using ColourCodepointMap = std::unordered_map<uint32_t, CodepointVector>;
    using FontColourCodepointMap = std::unordered_map<FT_Face, ColourCodepointMap>;
    using BackgroundRectVector = wxVector<struct __BackgroundRectAttrs>;

    wxScopedPtr<wxGraphicsContext> m_TextExtentContext;
    wxCoord m_GlyphAdvanceX;
    wxCoord m_LineHeight;
    TextPartVector m_TextParts;

    static fttb::FontManagerPtr m_FontManager;


    void PrepareTextRendering(FontColourCodepointMap & fcc_map, BackgroundRectVector & bg_rect_vector);

    void DoDrawText(wxGraphicsContext * context, const TextPart & text_part);

    wxString wxFontToFCDesc(const wxFont * pFont);
};
