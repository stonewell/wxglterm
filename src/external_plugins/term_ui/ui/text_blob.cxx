#include "text_blob.h"

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
}
