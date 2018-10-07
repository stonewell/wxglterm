#include "text_blob.h"
#include "wx/graphics.h"

void wxTextBlob::DoDrawText(wxGraphicsContext * context, const FontColourCodepointMap & fcc_map)
{
    (void)fcc_map;
    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        context->SetFont(*it->pFont, it->fore);
        context->DrawText(it->text, it->pt.x, it->pt.y);
    }
}
