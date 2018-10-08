#include "text_blob.h"
#include "wx/graphics.h"

void wxTextBlob::DoDrawText(wxGraphicsContext * context, void * rendering_data)
{
    (void)rendering_data;
    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        context->SetFont(*it->pFont, it->fore);
        context->DrawText(it->text, it->pt.x, it->pt.y);
    }
}

void wxTextBlob::DoDrawBackground(wxGraphicsContext * context, void * rendering_data) {
    const BackgroundRectVector & bg_rect_vector = *((BackgroundRectVector *)rendering_data);

    for(const auto & it : bg_rect_vector) {
        context->SetBrush(context->CreateBrush(wxBrush(it.back)));
        context->DrawRectangle(it.rect.GetX(), it.rect.GetY(), it.rect.GetWidth(), it.rect.GetHeight());
    }
}

void * wxTextBlob::BeginTextRendering() {
    BackgroundRectVector * bg_rect_vector = new BackgroundRectVector;

    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        if (it->back != wxNullColour) {
            bg_rect_vector->push_back({{it->pt, it->size}, it->back});
        }
    }

    return bg_rect_vector;
}

void wxTextBlob::EndTextRendering(void * rendering_data) {
    delete (BackgroundRectVector *)rendering_data;
}
