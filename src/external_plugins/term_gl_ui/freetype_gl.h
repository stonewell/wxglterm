#pragma once

#include "font-manager.h"
#include <memory>
#include <string>

enum FontCategoryEnum {
    Default = 0,
    Bold,
    Underlined,
    BoldUnderlined,

    FontCategoryCount
};

class freetype_gl_context {
public:
    freetype_gl_context();
    virtual ~freetype_gl_context();

    ftgl::font_manager_t * font_manager;
    std::string font_name;
    uint64_t font_size;
    float line_height;
    float col_width;
    std::string font_lang;

    ftgl::markup_t * get_font(FontCategoryEnum font_category);
    void init_font(const std::string & name, uint64_t size, const std::string & lang);

private:
    void cleanup();
    ftgl::markup_t fonts_markup[FontCategoryEnum::FontCategoryCount];
};

using freetype_gl_context_ptr = std::shared_ptr<freetype_gl_context>;

freetype_gl_context_ptr freetype_gl_init();
