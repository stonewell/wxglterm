#include "text_blob.h"
#include "wx/graphics.h"

#include "font_manager.h"
#include "char_width.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <forward_list>
#include <functional>

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

using Codepoints = std::forward_list<GlyphAttrs>;

typedef struct __ColourCodepoint {
    uint32_t colour;
    Codepoints code_points;
} ColourCodepoint;

using ColourCodepoints = std::forward_list<ColourCodepoint>;

typedef struct __SizeColourCodepoint {
    int size;
    ColourCodepoints colour_codepoints;
} SizeColourCodepoint;

using SizeColourCodepoints = std::forward_list<SizeColourCodepoint>;

typedef struct __FontSizeColourCodepoint {
    FT_Face font;
    SizeColourCodepoints size_colour_codepoints;
} FontSizeColourCodepoint;

using FontSizeColourCodepoints = std::forward_list<FontSizeColourCodepoint>;

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
    FontSizeColourCodepoints fcc;
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
    const FontSizeColourCodepoints & fcc_map = rdata->fcc;

    cairo_t * native_context = (cairo_t*)context->GetNativeContext();
    native_context = cairo_reference(native_context);

    for(const auto & it : fcc_map) {
        auto ct = cairo_ft_font_face_create_for_ft_face (it.font, 0);
        cairo_set_font_face (native_context, ct);

        for(const auto & it_size : it.size_colour_codepoints) {
            cairo_set_font_size (native_context, it_size.size * m_PPI.y / 72);

            cairo_font_extents_t fe;
            cairo_font_extents(native_context, &fe);

            for(const auto & it_color : it_size.colour_codepoints) {
                wxColour fore;
                fore.SetRGBA(it_color.colour);

                cairo_set_source_rgba(native_context,
                                      (double)fore.Red() / 255.0,
                                      (double)fore.Green() / 255.0,
                                      (double)fore.Blue() / 255.0,
                                      (double)fore.Alpha()/ 255.0);


                int glyph_index = 0;
                for(const auto & it_glyph : it_color.code_points) {
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

template<typename ForwardIterator>
ForwardIterator text_blob_find(ForwardIterator it_begin,
                               ForwardIterator it_end,
                               const std::function<bool(ForwardIterator)> & matcher) {
    while(it_begin != it_end) {
        if (matcher(it_begin))
            return it_begin;

        it_begin++;
    }

    return it_end;
}

void * wxTextBlob::BeginTextRendering() {
    RenderingData * rdata = new RenderingData;

    FontSizeColourCodepoints & fcc_map = rdata->fcc;
    BackgroundRectVector & bg_rect_vector = rdata->bg_rect_vector;

    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        std::function<bool(SizeColourCodepoints::iterator)> find_size{
            [it](SizeColourCodepoints::iterator it_cur) {
                return it_cur->size == it->pFont->GetPointSize();
            }
        };

        std::function<bool(ColourCodepoints::iterator)> find_colour{
            [it](ColourCodepoints::iterator it_cur) {
                return it_cur->colour == (uint32_t)it->fore.GetRGBA();
            }
        };

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

            if (ch == L' ') {
                if (m_GlyphAdvanceX == 0) {
                    pt.x = it->pt.x + extents[i];
                } else {
                    pt.x += m_GlyphAdvanceX;
                }
                continue;
            }

            FT_Face ft_face = font->EnsureGlyph(ch);

            std::function<bool(FontSizeColourCodepoints::iterator)> find_font{
                [ft_face](FontSizeColourCodepoints::iterator it_cur) {
                    return static_cast<void*>(ft_face) == static_cast<void*>(it_cur->font);
                }
            };

            auto it_font = text_blob_find(fcc_map.begin(),
                                          fcc_map.end(),
                                          find_font);

            if (it_font == fcc_map.end()) {
                fcc_map.push_front({});
                it_font = fcc_map.begin();
                it_font->font = ft_face;
            }

            auto it_size = text_blob_find(it_font->size_colour_codepoints.begin(),
                                          it_font->size_colour_codepoints.end(),
                                          find_size);

            if (it_size == it_font->size_colour_codepoints.end()) {
                it_font->size_colour_codepoints.push_front({});
                it_size = it_font->size_colour_codepoints.begin();
                it_size->size = it->pFont->GetPointSize();
            }

            auto it_colour = text_blob_find(it_size->colour_codepoints.begin(),
                                            it_size->colour_codepoints.end(),
                                            find_colour);

            if (it_colour == it_size->colour_codepoints.end()) {
                it_size->colour_codepoints.push_front({});
                it_colour = it_size->colour_codepoints.begin();
                it_colour->colour = (uint32_t)it->fore.GetRGBA();
            }

            auto char_index = FT_Get_Char_Index(ft_face, (FT_Long)ch);

            it_colour->code_points.push_front({ch, char_index, pt});

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
