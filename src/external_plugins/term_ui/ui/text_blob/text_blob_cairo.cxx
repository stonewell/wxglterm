#include "text_blob.h"
#include "wx/graphics.h"

#include <iostream>
#include <algorithm>
#include <vector>

#define cairo_public
#include <cairo.h>
#include <cairo-ft.h>

void wxTextBlob::DoDrawText(wxGraphicsContext * context, const FontColourCodepointMap & fcc_map)
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
