#include "text_blob.h"
#include "wx/graphics.h"

#include "font_manager.h"
#include "char_width.h"

#include <iostream>
#include <algorithm>
#include <vector>

#define cairo_public
#include <cairo.h>
#include <cairo-ft.h>

static
fttb::FontManagerPtr g_FontManager {fttb::CreateFontManager()};

static
wxString wxFontToFCDesc(const wxFont * pFont) {
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

typedef struct __GlyphAttrs {
    wchar_t codepoint;
    FT_UInt index;
    wxPoint pt;
} GlyphAttrs;

using CodepointVector = std::vector<GlyphAttrs>;
using ColourCodepointMap = std::unordered_map<uint32_t, CodepointVector>;
using SizeColourCodepointMap = std::unordered_map<int, ColourCodepointMap>;
using FontColourCodepointMap = std::unordered_map<FT_Face, SizeColourCodepointMap>;

/* cairo_show_glyphs accepts runs up to 102 glyphs before it allocates a
 * temporary array.
 *
 * Setting this to a large value can cause dramatic slow-downs for some
 * xservers (notably fglrx), see bug #410534.
 *
 * Moreover, setting it larger than %VTE_DRAW_MAX_LENGTH is nonsensical,
 * as the higher layers will not submit runs longer than that value.
 */
#define MAX_RUN_LENGTH 100

typedef struct __RenderingData {
    FontColourCodepointMap fcc_map;
    wxTextBlob::BackgroundRectVector bg_rect_vector;
    cairo_glyph_t glyphs[MAX_RUN_LENGTH];
} RenderingData;

void wxTextBlob::DoDrawBackground(wxGraphicsContext * context, void * rendering_data)
{
    RenderingData * rdata = (RenderingData*)rendering_data;
    const BackgroundRectVector & bg_rect_vector = rdata->bg_rect_vector;

    for(const auto & it : bg_rect_vector) {
        context->SetBrush(context->CreateBrush(wxBrush(it.back)));
        context->DrawRectangle(it.rect.GetX(), it.rect.GetY(), it.rect.GetWidth(), it.rect.GetHeight());
    }
}

void wxTextBlob::DoDrawText(wxGraphicsContext * context, void * rendering_data)
{
    RenderingData * rdata = (RenderingData*)rendering_data;
    const FontColourCodepointMap & fcc_map = rdata->fcc_map;

    cairo_t * native_context = (cairo_t*)context->GetNativeContext();
    native_context = cairo_reference(native_context);

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


                int glyph_index = 0;
                for(const auto & it_glyph : it_color.second) {
                    rdata->glyphs[glyph_index].index = it_glyph.index;
                    rdata->glyphs[glyph_index].x = (double)it_glyph.pt.x;
                    rdata->glyphs[glyph_index].y = (double)it_glyph.pt.y + fe.ascent;

                    glyph_index++;

                    if (glyph_index == MAX_RUN_LENGTH) {
                        cairo_show_glyphs(native_context, rdata->glyphs, MAX_RUN_LENGTH);
                        glyph_index = 0;
                    }
                }

                if (glyph_index > 0) {
                    cairo_show_glyphs(native_context, rdata->glyphs, glyph_index);
                }
            } //all color
        } //all font
    }

    cairo_destroy(native_context);
}

void * wxTextBlob::BeginTextRendering() {
    RenderingData * rdata = new RenderingData;

    FontColourCodepointMap & fcc_map = rdata->fcc_map;
    BackgroundRectVector & bg_rect_vector = rdata->bg_rect_vector;

    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        auto font = g_FontManager->CreateFontFromDesc(std::string(wxFontToFCDesc(it->pFont)));

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

            auto char_index = FT_Get_Char_Index(ft_face, (FT_Long)ch);

            p2.first->second.push_back({ch, char_index, pt});

            if (m_GlyphAdvanceX == 0) {
                pt.x = it->pt.x + extents[i];
            } else {
                pt.x += (char_width(ch) > 1 ? 2 * m_GlyphAdvanceX : m_GlyphAdvanceX);
            }
        }
    }

    return rdata;
}

void wxTextBlob::EndTextRendering(void * rendering_data) {
    RenderingData * rdata = (RenderingData *)rendering_data;

    delete rdata;
}
