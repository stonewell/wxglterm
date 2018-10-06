#include "text_blob.h"
#include "wx/dcgraph.h"
#include "wx/graphics.h"
#include "wx/tokenzr.h"

#include "char_width.h"

#include <iostream>
#include <algorithm>

fttb::FontManagerPtr wxTextBlob::m_FontManager {fttb::CreateFontManager()};

wxTextBlob::wxTextBlob()
    : m_TextExtentContext{wxGraphicsContext::Create()}
    , m_GlyphAdvanceX {0}
    , m_LineHeight {0}
    , m_TextParts {} {
      }

wxString wxTextBlob::wxFontToFCDesc(const wxFont * pFont) {
    wxString desc(pFont->GetFaceName());

    desc << ":size=" << pFont->GetPointSize();

    switch (pFont->GetStyle()) {
    case wxFONTSTYLE_ITALIC:
    case wxFONTSTYLE_SLANT:
        desc << ":slant=italic";
        break;
    default:
        break;
    }

    switch(pFont->GetWeight()){
    case wxFONTWEIGHT_BOLD:
        desc << ":weight=200";
        break;
    case wxFONTWEIGHT_LIGHT:
        desc << ":weight=50";
        break;
    default:
        break;
    }

    return desc;
}

wxPoint wxTextBlob::AddText(const wxString & text,
                            const wxPoint pt,
                            const wxFont * pFont,
                            wxColour fore_color,
                            wxColour back_color) {
    (void)text;
    (void)pt;
    (void)pFont;
    (void)fore_color;
    (void)back_color;

    m_TextExtentContext->SetFont(*pFont, fore_color);

    wxStringTokenizer tokenizer(text, "\n", wxTOKEN_RET_EMPTY_ALL);

    wxPoint tmpPt{pt}, lastPt;

    bool empty_token = false;

    do {
        auto token = tokenizer.HasMoreTokens() ? tokenizer.GetNextToken() : text;

        empty_token = token.IsEmpty();

        wxDouble width = 0.0, height = 0.0, leading = 0.0;

        m_TextExtentContext->GetTextExtent(empty_token ? " " : token,
                                           &width,
                                           &height,
                                           nullptr,
                                           &leading);

        if (empty_token) width = 0;

        m_TextParts.push_back(
            {token, tmpPt, pFont,
             fore_color, back_color,
             {(int)width, (int)std::max(height + leading, (wxDouble)m_LineHeight)}
            });

        lastPt = tmpPt;
        lastPt.x += width;

        tmpPt.x = pt.x;
        tmpPt.y += std::max(height + leading, (wxDouble)m_LineHeight);
    } while(tokenizer.HasMoreTokens());

    return empty_token ? tmpPt : lastPt;
}

void wxTextBlob::Render(wxGraphicsContext * context) {
    (void)context;

    FontColourCodepointMap fcc_map;
    PrepareFontGlyphs(fcc_map);

    context->PushState();
    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {
        context->SetFont(*it->pFont, it->fore);

        if (it->back != wxNullColour) {
            context->DrawText(it->text, it->pt.x, it->pt.y, context->CreateBrush(wxBrush(it->back)));
        } else {
            context->DrawText(it->text, it->pt.x, it->pt.y);
        }
    }
    context->PopState();
}

void wxTextBlob::PrepareFontGlyphs(FontColourCodepointMap & fcc_map) {
    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        m_TextExtentContext->SetFont(*it->pFont, it->fore);
        auto font = m_FontManager->CreateFontFromDesc(std::string(wxFontToFCDesc(it->pFont)));

        wxArrayDouble extents;

        if (m_GlyphAdvanceX == 0)
            m_TextExtentContext->GetPartialTextExtents(it->text, extents);

        wxWCharBuffer buf = it->text.wc_str();

        wxPoint pt = it->pt;
        for(size_t i=0; i < buf.length(); i++) {
            wchar_t ch = buf[i];

            FT_Face ft_face = font->EnsureGlyph(ch);

            auto p1 = fcc_map.insert(std::make_pair(ft_face, ColourCodepointMap{}));

            auto p2 = p1.first->second.insert(std::make_pair((uint32_t)it->fore.GetRGBA(), CodepointVector{}));

            if (m_GlyphAdvanceX == 0) {
                p2.first->second.push_back({ch, i == 0 ? it->pt : wxPoint(it->pt.x + extents[i - 1], it->pt.y)});
            } else {
                p2.first->second.push_back({ch, pt});
                pt.x += char_width(ch) > 1 ? 2 * m_GlyphAdvanceX : m_GlyphAdvanceX;
            }
        }
    }
}
