#include "freetype_gl.h"

#include <string.h>
#include <iostream>
#include <fontconfig/fontconfig.h>
#include <sstream>
#include <math.h>
#include <vector>

#define SINGLE_WIDTH_CHARACTERS                 \
    " !\"#$%&'()*+,-./"                         \
    "0123456789"                                \
    ":;<=>?@"                                   \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"                \
    "[\\]^_`"                                   \
    "abcdefghijklmnopqrstuvwxyz"                \
    "{|}~"

freetype_gl_context_ptr freetype_gl_init(const ftdgl::viewport::viewport_s & viewport) {
    freetype_gl_context_ptr ptr = std::make_shared<freetype_gl_context>(viewport);

    return ptr;
}

freetype_gl_context::freetype_gl_context(const ftdgl::viewport::viewport_s & viewport)
    : font_manager {ftdgl::CreateFontManager()}
    , font_name {"Monospace"}
    , font_size {48}
    , m_Viewport {viewport} {
        memset(fonts_markup, 0, sizeof(fonts_markup));
      }

freetype_gl_context::~freetype_gl_context() {
    cleanup();
}

void freetype_gl_context::cleanup() {
}

ftdgl::text::markup_s * freetype_gl_context::get_font(FontCategoryEnum font_category) {
    if (fonts_markup[font_category].font)
        return &fonts_markup[font_category];

    bool bold = (font_category == FontCategoryEnum::Bold || font_category == FontCategoryEnum::BoldUnderlined);

    std::stringstream ss;
    ss << font_name << ":size=" << font_size << ":lang=" << font_lang;

    if (bold)
        ss << ":weight=200";

    ftdgl::text::color_s white  = {1.0, 1.0, 1.0, 1.0};
    ftdgl::text::color_s none   = {1.0, 1.0, 1.0, 0.0};

    fonts_markup[font_category].fore_color    = white;
    fonts_markup[font_category].back_color    = none;

    fonts_markup[font_category].font = font_manager->CreateFontFromDesc(ss.str());

    return &fonts_markup[font_category];
}

void freetype_gl_context::init_font(const std::string & name,
                                    uint64_t size,
                                    const std::string & lang) {
    this->font_name = name;
    this->font_size = size;
    this->font_lang = lang;

    auto f = get_font(FontCategoryEnum::Default)->font;

    std::cout << "a:" << f->GetAscender()
              << ",d:" << f->GetDescender()
              << ",l:" << f->GetHeight()
              << std::endl;

    this->line_height = ceil(f->GetHeight()) + 1;
    this->col_width = 0.0f;

    for(size_t i=0;i < strlen(SINGLE_WIDTH_CHARACTERS); i++) {
        auto glyph = f->LoadGlyph(SINGLE_WIDTH_CHARACTERS[i]);

        if( glyph != NULL ) {
            auto glyph_width = glyph->GetAdvanceX();
            if (this->col_width < glyph_width) {
                this->col_width = glyph_width;
            }
        }
    }

    std::cout << "height:" << line_height << ", width:" << col_width << std::endl;

    reset_font_manager();
}

void freetype_gl_context::reset_font_manager() {
}
