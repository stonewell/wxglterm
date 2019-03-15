#include "text_blob.h"
#include "wx/graphics.h"
#include "wx/osx/core/cfstring.h"

#import <CoreText/CoreText.h>

#include <iostream>
#include <algorithm>
#include <vector>

#include "char_width.h"

#define USE_DELEGATE 0

#if USE_DELEGATE
typedef struct __DelegateCallbackData {
    CGFloat ascent;
    CGFloat descent;
    CGFloat adv_x;
} DelegateCallbackData;

/* Callbacks */
static
void MyDeallocationCallback( void* refCon ){
    (void)refCon;
}

static
CGFloat MyGetAscentCallback( void *refCon ){
    DelegateCallbackData * data = (DelegateCallbackData*)refCon;
    return data->ascent;
}

static
CGFloat MyGetDescentCallback( void *refCon ){
    DelegateCallbackData * data = (DelegateCallbackData*)refCon;
    return data->descent;
}

static
CGFloat MyGetWidthCallback( void* refCon ){
    DelegateCallbackData * data = (DelegateCallbackData*)refCon;
    return data->adv_x;
}
#endif

static
void __DoDrawText(CGContextRef cgContext, const wxTextBlob::TextPart & text_part, double adv_x);

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
        __DoDrawText(native_context, *it, m_GlyphAdvanceX);
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

#if !USE_DELEGATE
static
void __DrawLine(const wxString & str, CTLineRef line, CGContextRef cgContext, double adv_x);
#endif

static
void __DoDrawText(CGContextRef cgContext, const wxTextBlob::TextPart & text_part, double adv_x)
{
    wxCFStringRef text(text_part.text, wxLocale::GetSystemEncoding() );
    CTFontRef font = text_part.pFont->OSXGetCTFont();
    CGColorRef col = text_part.fore.CreateCGColor();

#if USE_DELEGATE
    DelegateCallbackData data {
        CTFontGetAscent(font),
        CTFontGetDescent(font),
        adv_x
    };

    CTRunDelegateCallbacks callbacks;
    callbacks.version = kCTRunDelegateVersion1;
    callbacks.dealloc = MyDeallocationCallback;
    callbacks.getAscent = MyGetAscentCallback;
    callbacks.getDescent = MyGetDescentCallback;
    callbacks.getWidth = MyGetWidthCallback;
    CTRunDelegateRef delegate = CTRunDelegateCreate(&callbacks, &data);
#endif

    CFStringRef keys[] = { kCTFontAttributeName , kCTForegroundColorAttributeName
#if USE_DELEGATE
                           , kCTRunDelegateAttributeName
#endif
    };

    CFTypeRef values[] = { font, col
#if USE_DELEGATE
                           , delegate
#endif
    };

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

#if USE_DELEGATE
        CTLineDraw(line, cgContext);
#else
    if (!adv_x)
        CTLineDraw(line, cgContext);
    else
        __DrawLine(text_part.text, line, cgContext, adv_x);
#endif

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

#if !USE_DELEGATE
static
void __ApplyStyles(CTRunRef run, CGContextRef context) {
    CFDictionaryRef attributes = CTRunGetAttributes(run);

    // Set the font
    CTFontRef runFont = (CTFontRef)CFDictionaryGetValue(attributes, kCTFontAttributeName);
    CGFontRef cgFont = CTFontCopyGraphicsFont(runFont, NULL);
    CGContextSetFont(context, cgFont);
    CGContextSetFontSize(context, CTFontGetSize(runFont));
    CFRelease(cgFont);

    // Set the color
    CGColorRef color = (CGColorRef)CFDictionaryGetValue(attributes, kCTForegroundColorAttributeName);
    CGContextSetFillColorWithColor(context, color);
}

static
void __DrawRun(const wxString & text, CTRunRef run, CGContextRef cgContext, double adv_x) {
    __ApplyStyles(run, cgContext);

    CFRange range = CTRunGetStringRange(run);
    wxWCharBuffer buf = text.wc_str();

    std::vector<CGPoint> positions;
    std::vector<CGGlyph> glyphs;

    CFDictionaryRef attributes = CTRunGetAttributes(run);
    CTFontRef runFont = (CTFontRef)CFDictionaryGetValue(attributes, kCTFontAttributeName);

    CGPoint pt = CGPointMake(0, 0);

    for(size_t i = 0; i < buf.length(); i++) {
        wchar_t ch = buf[(size_t)i];

        if ((CFIndex)i < range.location) {
            pt.x += (char_width(ch) > 1 ? adv_x * 2 : adv_x);
            continue;
        }

        if ((CFIndex)i >= range.location + range.length)
            break;

        CGGlyph glyph;
        CTFontGetGlyphsForCharacters(runFont, (UniChar*)&ch, &glyph, 1);

        positions.push_back(pt);
        glyphs.push_back(glyph);

        pt.x += (char_width(ch) > 1 ? adv_x * 2 : adv_x);
    }

    if (glyphs.size() > 0) {
        CGContextShowGlyphsAtPositions(cgContext, &glyphs[0], &positions[0], glyphs.size());
    }
}

static
void __DrawLine(const wxString & text, CTLineRef line, CGContextRef cgContext, double adv_x) {
    (void)line;
    (void)cgContext;
    (void)adv_x;

    CFArrayRef runs = CTLineGetGlyphRuns(line);

    for(CFIndex i=0;i < CFArrayGetCount(runs); i++) {
        CTRunRef run = (CTRunRef)CFArrayGetValueAtIndex(runs, i);

        __DrawRun(text, run, cgContext, adv_x);
    }
}
#endif
