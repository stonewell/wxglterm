#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void
term_text_buffer_printf(
#ifdef __cplusplus
    ftgl::
#endif
    text_buffer_t * self,
    double col_width,
#ifdef __cplusplus
    ftgl::
#endif
    vec2 *pen, ... );

#ifdef __cplusplus
}
#endif
