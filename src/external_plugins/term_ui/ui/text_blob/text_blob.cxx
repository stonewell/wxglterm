#include "text_blob.h"
#include "wx/dcgraph.h"
#include "wx/graphics.h"
#include "wx/tokenzr.h"

#include "char_width.h"

#include <iostream>
#include <algorithm>

wxTextBlob::wxTextBlob()
    : m_TextExtentContext{wxGraphicsContext::Create()}
    , m_GlyphAdvanceX {0}
    , m_LineHeight {0}
    , m_TextParts {}
    , m_PPI {72, 72} {
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

    void * rendering_data = BeginTextRendering();

    context->PushState();

    DoDrawBackground(context, rendering_data);
    DoDrawText(context, rendering_data);

    EndTextRendering(rendering_data);

    context->PopState();
}
