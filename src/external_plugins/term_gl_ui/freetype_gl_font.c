#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "distance-field.h"
#include "texture-font.h"
#include "platform.h"
#include "utf8-utils.h"
#include "texture-font.h"

void
texture_font_generate_kerning( texture_font_t *self,
                               FT_Library *library, FT_Face *face );

#define HRES  64
#define HRESf 64.f
#define DPI   72

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
static
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

// ------------------------------------------------- texture_font_load_face ---
static int
texture_font_load_face(texture_font_t *self, float size,
        FT_Library *library, FT_Face *face)
{
    FT_Error error;
    FT_Matrix matrix = {
        (int)((1.0/HRES) * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((0.0)      * 0x10000L),
        (int)((1.0)      * 0x10000L)};

    assert(library);
    assert(size);

    /* Initialize library */
    error = FT_Init_FreeType(library);
    if(error) {
        fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup;
    }

    /* Load face */
    switch (self->location) {
    case TEXTURE_FONT_FILE:
        error = FT_New_Face(*library, self->filename, 0, face);
        break;

    case TEXTURE_FONT_MEMORY:
        error = FT_New_Memory_Face(*library,
            self->memory.base, self->memory.size, 0, face);
        break;
    }

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_library;
    }

    /* Select charmap */
    error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_face;
    }

    /* Set char size */
    error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);

    if(error) {
        fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                __LINE__, FT_Errors[error].code, FT_Errors[error].message);
        goto cleanup_face;
    }

    /* Set transform matrix */
    FT_Set_Transform(*face, &matrix, NULL);

    return 1;

cleanup_face:
    FT_Done_Face( *face );
cleanup_library:
    FT_Done_FreeType( *library );
cleanup:
    return 0;
}

static
int
term_texture_font_load_glyph( texture_font_t * self,
                              FT_Library library,
                              FT_Face face,
                              FT_Int32 flags,
                              uint32_t codepoint)
{
    size_t i, x, y;

    FT_Error error;
    FT_Glyph ft_glyph;
    FT_GlyphSlot slot;
    FT_Bitmap ft_bitmap;

    FT_UInt glyph_index;
    texture_glyph_t *glyph;
    int ft_glyph_top = 0;
    int ft_glyph_left = 0;

    ivec4 region;

    flags = 0;
    ft_glyph_top = 0;
    ft_glyph_left = 0;
    glyph_index = FT_Get_Char_Index( face, (FT_ULong)codepoint );
    // WARNING: We use texture-atlas depth to guess if user wants
    //          LCD subpixel rendering

    error = FT_Load_Glyph( face, glyph_index, flags );
    if( error )
    {
        fprintf( stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
                 __LINE__, FT_Errors[error].code, FT_Errors[error].message );
        return 0;
    }

    if( self->rendermode == RENDER_NORMAL || self->rendermode == RENDER_SIGNED_DISTANCE_FIELD )
    {
        slot            = face->glyph;
        ft_bitmap       = slot->bitmap;
        ft_glyph_top    = slot->bitmap_top;
        ft_glyph_left   = slot->bitmap_left;
    }
    else
    {
        FT_Stroker stroker;
        FT_BitmapGlyph ft_bitmap_glyph;

        error = FT_Stroker_New( library, &stroker );

        if( error )
        {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                    FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        FT_Stroker_Set(stroker,
                        (int)(self->outline_thickness * HRES),
                        FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND,
                        0);

        error = FT_Get_Glyph( face->glyph, &ft_glyph);

        if( error )
        {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                    FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        if( self->rendermode == RENDER_OUTLINE_EDGE )
            error = FT_Glyph_Stroke( &ft_glyph, stroker, 1 );
        else if ( self->rendermode == RENDER_OUTLINE_POSITIVE )
            error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 0, 1 );
        else if ( self->rendermode == RENDER_OUTLINE_NEGATIVE )
            error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );

        if( error )
        {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                    FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        if( self->atlas->depth == 1 )
            error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        else
            error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_LCD, 0, 1);

        if( error )
        {
            fprintf(stderr, "FT_Error (0x%02x) : %s\n",
                    FT_Errors[error].code, FT_Errors[error].message);
            goto cleanup_stroker;
        }

        ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
        ft_bitmap       = ft_bitmap_glyph->bitmap;
        ft_glyph_top    = ft_bitmap_glyph->top;
        ft_glyph_left   = ft_bitmap_glyph->left;

cleanup_stroker:
        FT_Stroker_Done( stroker );

        if( error )
        {
            return 0;
        }
    }

    struct {
        int left;
        int top;
        int right;
        int bottom;
    } padding = { 0, 0, 1, 1 };

    if( self->rendermode == RENDER_SIGNED_DISTANCE_FIELD )
    {
        padding.top = 1;
        padding.left = 1;
    }

    size_t src_w = ft_bitmap.width/self->atlas->depth;
    size_t src_h = ft_bitmap.rows;

    size_t tgt_w = src_w + padding.left + padding.right;
    size_t tgt_h = src_h + padding.top + padding.bottom;

    region = texture_atlas_get_region( self->atlas, tgt_w, tgt_h );

    if ( region.x < 0 )
    {
        fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
        return 0;
    }

    x = region.x;
    y = region.y;

    unsigned char *buffer = calloc( tgt_w * tgt_h * self->atlas->depth, sizeof(unsigned char) );

    unsigned char *dst_ptr = buffer + (padding.top * tgt_w + padding.left) * self->atlas->depth;
    unsigned char *src_ptr = ft_bitmap.buffer;
    for( i = 0; i < src_h; i++ )
    {
        //difference between width and pitch: https://www.freetype.org/freetype2/docs/reference/ft2-basic_types.html#FT_Bitmap
        memcpy( dst_ptr, src_ptr, ft_bitmap.width);
        dst_ptr += tgt_w * self->atlas->depth;
        src_ptr += ft_bitmap.pitch;
    }

    if( self->rendermode == RENDER_SIGNED_DISTANCE_FIELD )
    {
        unsigned char *sdf = make_distance_mapb( buffer, tgt_w, tgt_h );
        free( buffer );
        buffer = sdf;
    }

    texture_atlas_set_region( self->atlas, x, y, tgt_w, tgt_h, buffer, tgt_w * self->atlas->depth);

    free( buffer );

    glyph = texture_glyph_new( );
    glyph->codepoint = codepoint;
    glyph->width    = tgt_w;
    glyph->height   = tgt_h;
    glyph->rendermode = self->rendermode;
    glyph->outline_thickness = self->outline_thickness;
    glyph->offset_x = ft_glyph_left;
    glyph->offset_y = ft_glyph_top;
    glyph->s0       = x/(float)self->atlas->width;
    glyph->t0       = y/(float)self->atlas->height;
    glyph->s1       = (x + glyph->width)/(float)self->atlas->width;
    glyph->t1       = (y + glyph->height)/(float)self->atlas->height;

    // Discard hinting to get advance
    FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
    slot = face->glyph;
    glyph->advance_x = slot->advance.x / HRESf;
    glyph->advance_y = slot->advance.y / HRESf;

    vector_push_back( self->glyphs, &glyph );

    if( self->rendermode != RENDER_NORMAL && self->rendermode != RENDER_SIGNED_DISTANCE_FIELD )
        FT_Done_Glyph( ft_glyph );

    texture_font_generate_kerning( self, &library, &face );

    return 1;
}

int
term_texture_font_load_glyphs( texture_font_t * self,
                              uint32_t * codepoints,
                              size_t len) {
    FT_Library library;
    FT_Face face;
    FT_Int32 flags = 0;

    if (!texture_font_load_face(self, self->size, &library, &face))
        return 0;

    if( self->rendermode != RENDER_NORMAL && self->rendermode != RENDER_SIGNED_DISTANCE_FIELD )
    {
        flags |= FT_LOAD_NO_BITMAP;
    }
    else
    {
        flags |= FT_LOAD_RENDER;
    }

    if( !self->hinting )
    {
        flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
    }
    else
    {
        flags |= FT_LOAD_FORCE_AUTOHINT;
    }

    if( self->atlas->depth == 3 )
    {
        FT_Library_SetLcdFilter( library, FT_LCD_FILTER_LIGHT );
        flags |= FT_LOAD_TARGET_LCD;

        if( self->filtering )
        {
            FT_Library_SetLcdFilterWeights( library, self->lcd_weights );
        }
    }

    for(size_t i=0; i < len; i++)
        term_texture_font_load_glyph(self,
                                     library,
                                     face,
                                     flags,
                                     codepoints[i]);

    FT_Done_Face( face );
    FT_Done_FreeType( library );
    return 1;
}
