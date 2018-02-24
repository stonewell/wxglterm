#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "Platform.h"

#include "term_window.h"
extern TermWindow * TermWindowFromEditor(void * wid);

#if 1
#define DGB_FUNC_CALLED {printf("file:%s func:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__);}
#else
#define DGB_FUNC_CALLED
#endif

namespace Scintilla {
class SurfaceImpl : public Surface {
public:
    WindowID m_Wid;
	SurfaceImpl() = default;
	virtual ~SurfaceImpl() override = default;

	void Init(WindowID wid) override {
        m_Wid = wid;
    }

	void Init(SurfaceID sid, WindowID wid) override {
        (void)sid;
        (void)wid;
    }

	void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override {
        (void)width;
        (void)height;
        (void)surface_;
        (void)wid;
    }

	void Clear() {
    }

	void Release() override {
    }

	bool Initialised() override {
        return true;
    }

	void PenColour(ColourDesired fore) override {
        (void)fore;
    }

	int LogPixelsY() override {
        return 0;
    }

	int DeviceHeightFont(int points) override {
        return points;
    }

	void MoveTo(int x_, int y_) override {
        (void)x_;
        (void)y_;
    }

	void LineTo(int x_, int y_) override {
        (void)x_;
        (void)y_;
    }

	void Polygon(Point *pts, int npts, ColourDesired fore, ColourDesired back) override {
        (void)pts;
        (void)npts;
        (void)fore;
        (void)back;
    }

	void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override {
        (void)rc;
        (void)fore;
        (void)back;
    }

	void FillRectangle(PRectangle rc, ColourDesired back) override {
        (void)rc;
        (void)back;
    }

	void FillRectangle(PRectangle rc, Surface &surfacePattern) override {
        (void)rc;
        (void)surfacePattern;
    }

	void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override {
        (void)rc;
        (void)fore;
        (void)back;
    }

	void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
                        ColourDesired outline, int alphaOutline, int flags) override {
        (void)rc;
        (void)cornerSize;
        (void)fill;
        (void)alphaFill;
        (void)outline;
        (void)alphaOutline;
        (void)flags;
    }

	void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) override {
        (void)rc;
        (void)width;
        (void)height;
        (void)pixelsImage;
    }

	void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override {
        (void)rc;
        (void)fore;
        (void)back;
    }

	void Copy(PRectangle rc, Point from, Surface &surfaceSource) override {
        (void)rc;
        (void)from;
        (void)surfaceSource;
    }

	void DrawTextBase(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore) {
        (void)rc;
        (void)font_;
        (void)ybase;
        (void)s;
        (void)len;
        (void)fore;
    }

	void DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override {
        (void)rc;
        (void)font_;
        (void)ybase;
        (void)s;
        (void)len;
        (void)fore;
        (void)back;
    }

	void DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore, ColourDesired back) override {
        (void)rc;
        (void)font_;
        (void)ybase;
        (void)s;
        (void)len;
        (void)fore;
        (void)back;
    }

	void DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, const char *s, int len, ColourDesired fore) override {
        (void)rc;
        (void)font_;
        (void)ybase;
        (void)s;
        (void)len;
        (void)fore;
    }
	void MeasureWidths(Font &font_, const char *s, int len, XYPOSITION *positions) override {
        (void)font_;
        (void)s;
        (void)len;
        (void)positions;
    }

	XYPOSITION WidthText(Font &font_, const char *s, int len) override {
        (void)font_;
        (void)s;
        (void)len;
        DGB_FUNC_CALLED;
        return 0;
    }

	XYPOSITION WidthChar(Font &font_, char ch) override {
        (void)font_;
        (void)ch;
        TermWindow * pWindow = TermWindowFromEditor(m_Wid);

        if (pWindow)
            return pWindow->GetColWidth();
        return 0;
    }

	XYPOSITION Ascent(Font &font_) override {
        (void)font_;
        TermWindow * pWindow = TermWindowFromEditor(m_Wid);

        if (pWindow)
            return pWindow->GetLineHeight();
        return 0;
    }

	XYPOSITION Descent(Font &font_) override {
        (void)font_;
        return 0;
    }

	XYPOSITION InternalLeading(Font &font_) override {
        (void)font_;
        return 0;
    }

	XYPOSITION Height(Font &font_) override {
        (void)font_;
        TermWindow * pWindow = TermWindowFromEditor(m_Wid);

        if (pWindow)
            return pWindow->GetLineHeight();
        return 0;
    }

	XYPOSITION AverageCharWidth(Font &font_) override {
        (void)font_;
        TermWindow * pWindow = TermWindowFromEditor(m_Wid);

        if (pWindow)
            return pWindow->GetColWidth();
        return 0;
    }

	void SetClip(PRectangle rc) override {
        (void)rc;
    }

	void FlushCachedState() override {
    }

	void SetUnicodeMode(bool unicodeMode_) override {
        (void)unicodeMode_;
    }

	void SetDBCSMode(int codePage) override {
        (void)codePage;
    }
};

Surface *Surface::Allocate(int) {
    return new SurfaceImpl;
}
}
