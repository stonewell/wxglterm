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
    // wxCHECK_RET( !text_part.pFont->IsNull(),
    //              wxT("DoDrawText - no valid font set") );

    // if (text_part.text.empty())
    //     return;

    // const wxCharBuffer data = text_part.text.utf8_str();
    // if ( !data )
    //     return;

    // cairo_t * native_context = (cairo_t*)context->GetNativeContext();

    // //Apply font properties
    // cairo_set_source_rgba(native_context,
    //                       text_part.fore.Red(),
    //                       text_part.fore.Green(),
    //                       text_part.fore.Blue(),
    //                       text_part.fore.Alpha());

    // // Cairo's x,y for drawing text is at the baseline, so we need to adjust
    // // the position we move to by the ascent.
    // cairo_font_extents_t fe;
    // cairo_font_extents(native_context, &fe);
    // cairo_move_to(native_context, text_part.pt.x, text_part.pt.y+fe.ascent);

    // cairo_show_text(native_context, data);
    (void)context;
    (void)fcc_map;

    cairo_t * native_context = (cairo_t*)context->GetNativeContext();

    for(const auto & it : fcc_map) {
        auto ct = cairo_ft_font_face_create_for_ft_face (it.first, 0);
        cairo_set_font_face (native_context, ct);

        for(const auto & it_size : it.second) {
            cairo_set_font_size (native_context, it_size.first);

            for(const auto & it_color : it_size.second) {
                wxColour fore;
                fore.SetRGBA(it_color.first);

                cairo_set_source_rgba(native_context,
                                      fore.Red(),
                                      fore.Green(),
                                      fore.Blue(),
                                      fore.Alpha());

                std::vector<cairo_glyph_t> glyphs(it_color.second.size());

                for(const auto & it_glyph : it_color.second) {
                    glyphs.push_back({it_glyph.index,
                                      (double)it_glyph.pt.x,
                                      (double)it_glyph.pt.y});
                }

                cairo_show_glyphs(native_context, &glyphs[0], glyphs.size());
            }
        }
    }
}
