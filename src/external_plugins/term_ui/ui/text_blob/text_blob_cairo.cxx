#include "text_blob_cairo.h"
#include "wx/graphics.h"

#include <iostream>
#include <algorithm>
#include <vector>

#define cairo_public
#include <cairo.h>
#include <cairo-ft.h>

fttb::FontManagerPtr wxTextBlob::m_FontManager {fttb::CreateFontManager()};

wxString wxTextBlobCairo::wxFontToFCDesc(const wxFont * pFont) {
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

void wxTextBlobCairo::DoDrawText(wxGraphicsContext * context, const FontColourCodepointMap & fcc_map)
{
    cairo_t * native_context = (cairo_t*)context->GetNativeContext();

    for(const auto & it : fcc_map) {
        auto ct = cairo_ft_font_face_create_for_ft_face (it.first, 0);
        cairo_set_font_face (native_context, ct);
        for(const auto & it_size : it.second) {
            cairo_set_font_size (native_context, it_size.first * m_PPI.y / 72);

            cairo_font_extents_t fe;
            cairo_font_extents(native_context, &fe);

            for(const auto & it_color : it_size.second) {
                wxColour fore;
                fore.SetRGBA(it_color.first);

                cairo_set_source_rgba(native_context,
                                      (double)fore.Red() / 255.0,
                                      (double)fore.Green() / 255.0,
                                      (double)fore.Blue() / 255.0,
                                      (double)fore.Alpha()/ 255.0);

                std::vector<cairo_glyph_t> glyphs(it_color.second.size());

                for(const auto & it_glyph : it_color.second) {
                    glyphs.push_back({it_glyph.index,
                                      (double)it_glyph.pt.x,
                                      (double)it_glyph.pt.y + fe.ascent});
                }

                cairo_show_glyphs(native_context, &glyphs[0], glyphs.size());
            }
        }
    }
}

void wxTextBlobCairo::PrepareTextRendering(FontColourCodepointMap & fcc_map, BackgroundRectVector & bg_rect_vector) {
    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        std::string desc(std::string(wxFontToFCDesc(it->pFont)));

        auto font = m_FontManager->CreateFontFromDesc(desc);

        if (it->back != wxNullColour) {
            bg_rect_vector.push_back({{it->pt, it->size}, it->back});
        }

        wxArrayDouble extents;

        if (m_GlyphAdvanceX == 0) {
            m_TextExtentContext->SetFont(*it->pFont, it->fore);
            m_TextExtentContext->GetPartialTextExtents(it->text, extents);
        }

        wxWCharBuffer buf = it->text.wc_str();

        wxPoint pt = it->pt;

        for(size_t i=0; i < buf.length(); i++) {
            wchar_t ch = buf[i];

            FT_Face ft_face = font->EnsureGlyph(ch);

            auto p0 = fcc_map.insert(std::make_pair(ft_face,
                                                    SizeColourCodepointMap{}));

            auto p1 = p0.first->second.insert(std::make_pair(it->pFont->GetPointSize(),
                                                             ColourCodepointMap{}));

            auto p2 = p1.first->second.insert(std::make_pair((uint32_t)it->fore.GetRGBA(),
                                                             CodepointVector{}));

            p2.first->second.push_back({ch, FT_Get_Char_Index(ft_face, (FT_Long)ch), pt});

            if (m_GlyphAdvanceX == 0) {
                pt.x = it->pt.x + extents[i];
            } else {
                pt.x += (char_width(ch) > 1 ? 2 * m_GlyphAdvanceX : m_GlyphAdvanceX);
            }
        }
    }
}
