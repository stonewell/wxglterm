#include "freetype_gl.h"
#include "text-buffer.h"
#include "texture-font.h"
#include "texture-atlas.h"

#include <string.h>
#include <iostream>
#include <fontconfig/fontconfig.h>
#include <sstream>
#include <math.h>
#include <vector>

#include "freetype_gl_font.h"

#define SINGLE_WIDTH_CHARACTERS                 \
    " !\"#$%&'()*+,-./"                         \
    "0123456789"                                \
    ":;<=>?@"                                   \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"                \
    "[\\]^_`"                                   \
    "abcdefghijklmnopqrstuvwxyz"                \
    "{|}~"

freetype_gl_context_ptr freetype_gl_init() {
    freetype_gl_context_ptr ptr = std::make_shared<freetype_gl_context>();

    return ptr;
}

freetype_gl_context::freetype_gl_context()
    : font_manager {ftgl::font_manager_new(256, 256, LCD_FILTERING_ON)}
    , font_name {"Mono"}
    , font_size {48}
    , atlas_line_char_count {32} {
        memset(fonts_markup, 0, sizeof(fonts_markup));
      }

freetype_gl_context::~freetype_gl_context() {
    cleanup();

    ftgl::font_manager_delete(font_manager);
}

void freetype_gl_context::cleanup() {
    for(int i=0;i < FontCategoryEnum::FontCategoryCount;i++) {
        if (fonts_markup[i].family)
            free(fonts_markup[i].family);

        memset(&fonts_markup[i], 0, sizeof(ftgl::markup_t));
    }
}

static
char *
match_description(const char * description )
{

#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
    fprintf( stderr, "\"font_manager_match_description\" "
             "not implemented for windows.\n" );
    return 0;
#endif

    char *filename = 0;
    FcInit();
    FcPattern *pattern = FcNameParse((const FcChar8 *)description);
    if (!pattern)
    {
        fprintf( stderr, "fontconfig error: could not match description '%s'\n", description );
        return 0;
    }

    FcConfigSubstitute( 0, pattern, FcMatchPattern );
    FcDefaultSubstitute( pattern );
    FcResult result;
    FcPattern *match = FcFontMatch( 0, pattern, &result );
    FcPatternDestroy( pattern );

    if ( !match )
    {
        fprintf( stderr, "fontconfig error: could not match description '%s'\n", description );
        return 0;
    }
    else
    {
        FcValue value;
        FcResult result = FcPatternGet( match, FC_FILE, 0, &value );
        if ( result )
        {
            fprintf( stderr, "fontconfig error: could not match description '%s'", description );
        }
        else
        {
            filename = strdup( (char *)(value.u.s) );
        }
    }
    FcPatternDestroy( match );
    return filename;
}

ftgl::markup_t * freetype_gl_context::get_font(FontCategoryEnum font_category) {
    if (fonts_markup[font_category].font)
        return &fonts_markup[font_category];

    bool bold = (font_category == FontCategoryEnum::Bold || font_category == FontCategoryEnum::BoldUnderlined);

    std::stringstream ss;
    ss << font_name << "-" << font_size << ":lang=" << font_lang;

    if (bold)
        ss << ":weight=200";

    ftgl::vec4 white  = {{1.0, 1.0, 1.0, 1.0}};
    ftgl::vec4 none   = {{1.0, 1.0, 1.0, 0.0}};

    char * family = match_description(ss.str().c_str());

    std::cout << "matched family:" << family << ", desc:" << ss.str() << std::endl;

    fonts_markup[font_category].family  = family;
    fonts_markup[font_category].size    = (float)font_size;
    fonts_markup[font_category].bold    = bold;
    fonts_markup[font_category].italic  = 0;
    fonts_markup[font_category].spacing = 0.0;
    fonts_markup[font_category].gamma   = 2.;
    fonts_markup[font_category].foreground_color    = white;
    fonts_markup[font_category].background_color    = none;
    fonts_markup[font_category].underline           = (font_category == FontCategoryEnum::Underlined || font_category == FontCategoryEnum::BoldUnderlined);
    fonts_markup[font_category].underline_color     = white;
    fonts_markup[font_category].overline            = 0;
    fonts_markup[font_category].overline_color      = white;
    fonts_markup[font_category].strikethrough       = 0;
    fonts_markup[font_category].strikethrough_color = white;

    fonts_markup[font_category].font =
            font_manager_get_from_markup(font_manager,
                                         &fonts_markup[font_category]);
    fonts_markup[font_category].font->kerning = 0.0f;

    return &fonts_markup[font_category];
}

void freetype_gl_context::init_font(const std::string & name, uint64_t size, const std::string & lang) {
    this->font_name = name;
    this->font_size = size;
    this->font_lang = lang;

    ftgl::texture_font_t * f = get_font(FontCategoryEnum::Default)->font;

    std::cout << "a:" << f->ascender
              << ",d:" << f->descender
              << ",l:" << f->linegap
              << ",h:" << f->height
              << "," << f->filename
              << std::endl;

    this->line_height = ceil(f->height) + 1;
    this->col_width = 0.0f;

    (void)f;
    for(size_t i=0;i < strlen(SINGLE_WIDTH_CHARACTERS); i++) {
        ftgl::texture_glyph_t *glyph =
                texture_font_get_glyph(f,
                                       &SINGLE_WIDTH_CHARACTERS[i]);

        if( glyph != NULL ) {
            if (this->col_width < glyph->advance_x) {
                this->col_width = glyph->advance_x;
            }
        }
    }

    std::cout << "height:" << line_height << ", width:" << col_width << std::endl;

    reset_font_manager();
}

void freetype_gl_context::reset_font_manager() {
    cleanup();

    ftgl::font_manager_delete(font_manager);
    font_manager = ftgl::font_manager_new(atlas_line_char_count * (ceil(col_width) * 2),
                                          atlas_line_char_count * (line_height),
                                          LCD_FILTERING_ON);
}

void freetype_gl_context::ensure_glyphs(const std::unordered_set<uint32_t> & codepoints) {
    ftgl::texture_font_t * f = get_font(FontCategoryEnum::Default)->font;

    size_t i;
    ftgl::texture_glyph_t *glyph;

    std::unordered_set<uint32_t> tmp{codepoints};

    for( i = 0; i < f->glyphs->size; ++i )
    {
        glyph = *(ftgl::texture_glyph_t **) vector_get( f->glyphs, i );
        tmp.erase(glyph->codepoint);
    }

    int used_glyphs = 0;
    for(int i=0;i < FontCategoryEnum::FontCategoryCount;i++) {
        if (fonts_markup[i].font) {
            used_glyphs += fonts_markup[i].font->glyphs->size;
        }
    }

    // std::cout << tmp.size()
    //           << "," << atlas_line_char_count
    //           << "," << used_glyphs
    //           << "," << codepoints.size()
    //           << std::endl;
    if (tmp.size() + f->glyphs->size <= atlas_line_char_count * atlas_line_char_count)
        return;

    if (atlas_line_char_count < ceil(sqrt((float)codepoints.size())))
        atlas_line_char_count = std::max(atlas_line_char_count * 2, ceil(sqrt((float)codepoints.size())));

    std::cout << "line_char_count:" << atlas_line_char_count << std::endl;

    reset_font_manager();

    // f = get_font(FontCategoryEnum::Default)->font;

    // std::vector<uint32_t> values;
    // values.insert(values.begin(), codepoints.begin(), codepoints.end());

    // term_texture_font_load_glyphs(f,
    //                               &values[0],
    //                               values.size());
}
