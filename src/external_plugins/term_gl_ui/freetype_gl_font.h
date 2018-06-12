#ifndef __FREETYPE_GL_FONT_H__
#define __FREETYPE_GL_FONT_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
namespace ftgl {
#endif

int
term_texture_font_load_glyphs( texture_font_t * self,
                               uint32_t * codepoints,
                               size_t len);

#ifdef __cplusplus
}
}
#endif // ifdef __cplusplus

#endif
