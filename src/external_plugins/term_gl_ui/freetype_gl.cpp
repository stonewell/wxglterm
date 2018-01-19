#include "freetype_gl.h"

freetype_gl_context_ptr freetype_gl_init() {
    freetype_gl_context_ptr ptr = std::make_shared<freetype_gl_context>();

    return ptr;
}

freetype_gl_context::freetype_gl_context()
    : font_manager {nullptr}
    , font_name {"Mono"}
    , font_size {16} {
        memset(fonts_markup, 0, sizeof(fonts_markup));
      }

freetype_gl_context::~freetype_gl_context() {
    cleanup();
}

void freetype_gl_context::cleanup() {
    for(int i=0;i < FontCategoryEnum::FontCategoryCount;i++) {
        if (fonts_markup[i].font)
            font_manager_delete_font(font_manager, fonts_markup[i].font);
        if (fonts_markup[i].family)
            free(fonts_markup[i].family);

        memset(&fonts_markup[i], 0, sizeof(ftgl::markup_t));
    }
}

ftgl::texture_font_t * freetype_gl_context::get_font(FontCategoryEnum font_category) {
    if (fonts_markup[font_category].font)
        return fonts_markup[font_category].font;

    fonts_markup[font_category] = {
        .family = strdup(font_name.c_str()),
        .size = (float)font_size,
        .bold = (font_category == FontCategoryEnum::Bold || font_category == FontCategoryEnum::BoldUnderlined),
        .underline = (font_category == FontCategoryEnum::Underlined || font_category == FontCategoryEnum::BoldUnderlined)
    };

    fonts_markup[font_category].font =
            font_manager_get_from_markup(font_manager,
                                         &fonts_markup[font_category]);

    return fonts_markup[font_category].font;
}

void freetype_gl_context::init_font(const std::string & name, uint64_t size) {
    this->font_name = name;
    this->font_size = size;

    cleanup();
}
