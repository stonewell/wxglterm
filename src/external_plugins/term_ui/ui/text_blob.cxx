#include "text_blob.h"
#include "wx/dcgraph.h"
#include "wx/graphics.h"

#include <iostream>

wxTextBlob::wxTextBlob()
    : m_GlyphAdvanceX {0} {
}

void wxTextBlob::AddText(const wxString & text,
                         const wxPoint pt,
                         const wxFont * pFont,
                         wxColour fore_color,
                         wxColour back_color) {
    (void)text;
    (void)pt;
    (void)pFont;
    (void)fore_color;
    (void)back_color;

    m_TextParts.push_back({text, pt, pFont, fore_color, back_color});
}

void wxTextBlob::Render(wxGraphicsContext * context) {
    (void)context;

    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {
        context->SetFont(*it->pFont, it->fore);
        context->SetBrush(wxBrush(it->back));
        context->DrawText(it->text, it->pt.x, it->pt.y);
    }
}
