#pragma once

#if defined(__APPLE__)
#   include "TargetConditionals.h"
#   if TARGET_OS_SIMULATOR || TARGET_OS_IPHONE
#     if defined(FREETYPE_GL_ES_VERSION_3_0)
#       include <OpenGLES/ES3/gl.h>
#     else
#       include <OpenGLES/ES2/gl.h>
#     endif
#   else
#     include <OpenGL/gl.h>
#   endif
#elif defined(_WIN32) || defined(_WIN64)
#  include <GL/glew.h>
#  include <GL/wglew.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#include "font_manager.h"
#include "text_buffer.h"
#include "viewport.h"

#include <memory>
#include <string>
#include <unordered_set>

enum FontCategoryEnum {
    Default = 0,
    Bold,
    Underlined,
    BoldUnderlined,

    FontCategoryCount
};

class freetype_gl_context {
public:
    freetype_gl_context(const ftdgl::viewport::viewport_s & viewport);
    virtual ~freetype_gl_context();

    ftdgl::FontManagerPtr font_manager;
    std::string font_name;
    uint64_t font_size;
    float line_height;
    float col_width;
    std::string font_lang;
    const ftdgl::viewport::viewport_s & m_Viewport;

    ftdgl::text::markup_s * get_font(FontCategoryEnum font_category);
    void init_font(const std::string & name, uint64_t size, const std::string & lang);
private:
    void cleanup();
    void reset_font_manager();
    ftdgl::text::markup_s fonts_markup[FontCategoryEnum::FontCategoryCount];
};

using freetype_gl_context_ptr = std::shared_ptr<freetype_gl_context>;

freetype_gl_context_ptr freetype_gl_init(const ftdgl::viewport::viewport_s & viewport);
