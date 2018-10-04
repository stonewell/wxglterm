#include "text_blob.h"
#include "wx/graphics.h"

#include <iostream>
#include <algorithm>

#define cairo_public
#include <cairo.h>

void wxTextBlob::DoDrawText(wxGraphicsContext * context, const TextPart & text_part)
{
    wxCHECK_RET( !text_part.pFont->IsNull(),
                 wxT("DoDrawText - no valid font set") );

    if (text_part.text.empty())
        return;

    const wxCharBuffer data = text_part.text.utf8_str();
    if ( !data )
        return;

    cairo_t * native_context = (cairo_t*)context->GetNativeContext();

    //Apply font properties
    cairo_set_source_rgba(native_context,
                          text_part.fore.Red(),
                          text_part.fore.Green(),
                          text_part.fore.Blue(),
                          text_part.fore.Alpha());

    // Cairo's x,y for drawing text is at the baseline, so we need to adjust
    // the position we move to by the ascent.
    cairo_font_extents_t fe;
    cairo_font_extents(native_context, &fe);
    cairo_move_to(native_context, text_part.pt.x, text_part.pt.y+fe.ascent);

    cairo_show_text(native_context, data);
}
