#include "text_blob.h"
#include "wx/dcgraph.h"
#include "wx/graphics.h"
#include "wx/tokenzr.h"

#include <iostream>
#include <algorithm>

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

    std::cout << "font:" << pFont->GetNativeFontInfoUserDesc() << "," << pFont->GetPointSize() << "," << pFont->GetPixelSize().y << pFont->GetWeight()
              << wxFontToFCDesc(pFont) << std::endl;

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
