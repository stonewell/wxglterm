#include "text_blob.h"
#include "wx/graphics.h"
#include "wx/osx/core/cfstring.h"

#import <CoreText/CoreText.h>

#include <iostream>
#include <algorithm>
#include <vector>

static
void __DoDrawText(CGContextRef cgContext, const wxTextBlob::TextPart & text_part);

void wxTextBlob::DoDrawBackground(wxGraphicsContext * context, void * rendering_data)
{
    const BackgroundRectVector & bg_rect_vector = *(BackgroundRectVector*)rendering_data;

    for(const auto & it : bg_rect_vector) {
        context->SetBrush(context->CreateBrush(wxBrush(it.back)));
        context->DrawRectangle(it.rect.GetX(), it.rect.GetY(), it.rect.GetWidth(), it.rect.GetHeight());
    }
}

void wxTextBlob::DoDrawText(wxGraphicsContext * context, void * rendering_data)
{
    (void)rendering_data;
    CGContextRef native_context = (CGContextRef)context->GetNativeContext();

    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {
        __DoDrawText(native_context, *it);
    }
}

void * wxTextBlob::BeginTextRendering() {
    BackgroundRectVector * rdata = new BackgroundRectVector;

    BackgroundRectVector & bg_rect_vector = *rdata;

    for(auto it = m_TextParts.begin(),
                it_end = m_TextParts.end();
        it != it_end;
        it++) {

        if (it->back != wxNullColour) {
            bg_rect_vector.push_back({{it->pt, it->size}, it->back});
        }
    }

    return rdata;
}

void wxTextBlob::EndTextRendering(void * rendering_data) {
    BackgroundRectVector * rdata = (BackgroundRectVector *)rendering_data;

    delete rdata;
}

static
void __DoDrawText(CGContextRef cgContext, const wxTextBlob::TextPart & text_part)
{
    wxCFStringRef text(text_part.text, wxLocale::GetSystemEncoding() );
    CTFontRef font = text_part.pFont->OSXGetCTFont();
    CGColorRef col = text_part.fore.CreateCGColor();
    CFStringRef keys[] = { kCTFontAttributeName , kCTForegroundColorAttributeName };
    CFTypeRef values[] = { font, col };

    wxCFRef<CFDictionaryRef> attributes( CFDictionaryCreate(kCFAllocatorDefault, (const void**) &keys, (const void**) &values,
                                                    WXSIZEOF( keys ), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks) );
    wxCFRef<CFAttributedStringRef> attrtext( CFAttributedStringCreate(kCFAllocatorDefault, text, attributes) );
    wxCFRef<CTLineRef> line( CTLineCreateWithAttributedString(attrtext) );

    CGFloat x = (CGFloat)text_part.pt.x;
    CGFloat y = (CGFloat)text_part.pt.y;

    y += CTFontGetAscent(font);

    CGContextSaveGState(cgContext);
    CGAffineTransform textMatrix = CGContextGetTextMatrix(cgContext);

    CGContextTranslateCTM(cgContext, (CGFloat)x, (CGFloat)y);
    CGContextScaleCTM(cgContext, 1, -1);
    CGContextSetTextMatrix(cgContext, CGAffineTransformIdentity);

    CTLineDraw( line, cgContext );

    if ( text_part.pFont->GetUnderlined() ) {
        //AKT: draw horizontal line 1 pixel thick and with 1 pixel gap under baseline
        CGFloat width = CTLineGetTypographicBounds(line, NULL, NULL, NULL);

        CGPoint points[] = { {0.0, -2.0},  {width, -2.0} };

        CGContextSetStrokeColorWithColor(cgContext, col);
        CGContextSetShouldAntialias(cgContext, false);
        CGContextSetLineWidth(cgContext, 1.0);
        CGContextStrokeLineSegments(cgContext, points, 2);
    }
    if ( text_part.pFont->GetStrikethrough() )
    {
        CGFloat width = CTLineGetTypographicBounds(line, NULL, NULL, NULL);
        CGFloat height = CTFontGetXHeight( font );
        CGPoint points[] = { {0.0, height * 0.6},  {width, height * 0.6} };
        CGContextSetStrokeColorWithColor(cgContext, col);
        CGContextSetShouldAntialias(cgContext, false);
        CGContextSetLineWidth(cgContext, 1.0);
        CGContextStrokeLineSegments(cgContext, points, 2);
    }

    CGContextRestoreGState(cgContext);
    CGContextSetTextMatrix(cgContext, textMatrix);
    CGColorRelease( col );
}
