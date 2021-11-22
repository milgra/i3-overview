#ifndef text_h
#define text_h

#include "zc_bitmap.c"
#include "zc_string.c"
#include "zc_vector.c"

#include <stdint.h>

typedef enum _textalign_t
{
  TA_LEFT,
  TA_CENTER,
  TA_RIGHT,
  TA_JUSTIFY,
} textalign_t;

typedef enum _vertalign_t
{
  VA_CENTER,
  VA_TOP,
  VA_BOTTOM,
} vertalign_t;

typedef enum _autosize_t
{
  AS_FIX,
  AS_AUTO,
} autosize_t;

typedef struct _textstyle_t
{
  char*       font;
  textalign_t align;
  vertalign_t valign;
  autosize_t  autosize;
  char        multiline;

  float size;
  int   line_height;
  int   margin;
  int   margin_top;
  int   margin_right;
  int   margin_bottom;
  int   margin_left;

  uint32_t textcolor;
  uint32_t backcolor;
} textstyle_t;

typedef struct _glyph_t
{
  int      x;
  int      y;
  int      w;
  int      h;
  float    x_scale;
  float    y_scale;
  float    x_shift;
  float    y_shift;
  float    asc;
  float    desc;
  float    base_y;
  uint32_t cp;
} glyph_t;

void text_init();
void text_destroy();

void text_break_glyphs(
    glyph_t*    glyphs,
    int         count,
    textstyle_t style,
    int         wth,
    int         hth,
    int*        nwth,
    int*        nhth);

void text_align_glyphs(glyph_t*    glyphs,
                       int         count,
                       textstyle_t style,
                       int         w,
                       int         h);

void text_render_glyph(glyph_t     g,
                       textstyle_t style,
                       bm_t*       bitmap);

void text_render_glyphs(glyph_t*    glyphs,
                        int         count,
                        textstyle_t style,
                        bm_t*       bitmap);

void text_layout(glyph_t*    glyphs,
                 int         count,
                 textstyle_t style,
                 int         wth,
                 int         hth,
                 int*        nwth,
                 int*        nhth);

void text_render(
    str_t*      text,
    textstyle_t style,
    bm_t*       bitmap);

void text_measure(str_t* text, textstyle_t style, int w, int h, int* nw, int* nh);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "zc_graphics.c"
#include "zc_map.c"
#include "zm_math2.c"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

struct _txt_t
{
  map_t*         fonts;
  unsigned char* gbytes; // byte array for glyph baking
  size_t         gcount; // byte array size for glyph baking
} txt;

void text_init()
{
  txt.fonts  = MNEW();                        // GREL 0
  txt.gbytes = malloc(sizeof(unsigned char)); // GREL 1
  txt.gcount = 1;
}

void text_destroy()
{
  vec_t* fonts = VNEW(); // REL 0
  map_values(txt.fonts, fonts);

  for (int i = 0; i < fonts->length; i++)
  {
    stbtt_fontinfo* font = fonts->data[i];
    free(font->data); // GREL 2
  }

  REL(fonts);       // REL 0
  REL(txt.fonts);   // GREL 0
  free(txt.gbytes); // GREL 1
}

void text_font_load(char* path)
{
  assert(txt.fonts != NULL);

  struct stat filestat;
  char        succ = stat(path, &filestat);
  if (succ == 0 && filestat.st_size > 0)
  {
    FILE* file = fopen(path, "rb"); // CLOSE 0

    if (file)
    {
      unsigned char* data = malloc((size_t)filestat.st_size); // GREL 2
      fread(data, (size_t)filestat.st_size, 1, file);

      stbtt_fontinfo font;
      stbtt_InitFont(&font, data, stbtt_GetFontOffsetForIndex(data, 0));

      MPUTR(txt.fonts, path, HEAP(font));

      printf("Font loaded %s txt.fonts in file %i\n", path, stbtt_GetNumberOfFonts(data));

      fclose(file); // CLOSE 0
    }
    else
      printf("cannot open font %s\n", path);
  }
  else
    printf("cannot open font %s\n", path);
}

// breaks text into lines in given rect
// first step for further alignment
void text_break_glyphs(
    glyph_t*    glyphs,
    int         count,
    textstyle_t style,
    int         wth,
    int         hth,
    int*        nwth,
    int*        nhth)
{

  stbtt_fontinfo* font = MGET(txt.fonts, style.font);
  if (font == NULL)
  {
    text_font_load(style.font);
    font = MGET(txt.fonts, style.font);
    if (!font) return;
  }

  int   spc_a, spc_p;      // actual space, prev space
  int   asc, desc, lgap;   // glyph ascent, descent, linegap
  int   lsb, advx;         // left side bearing, glyph advance
  int   fonth, lineh;      // base height, line height
  float scale, xpos, ypos; // font scale, position

  stbtt_GetFontVMetrics(font, &asc, &desc, &lgap);
  scale = stbtt_ScaleForPixelHeight(font, style.size);

  fonth = asc - desc;
  lineh = fonth + lgap;

  spc_a = 0;
  spc_p = 0;
  xpos  = 0;
  ypos  = (float)asc * scale;

  for (int index = 0; index < count; index++)
  {
    glyph_t glyph = glyphs[index];

    int cp  = glyphs[index].cp;
    int ncp = index < count - 1 ? glyphs[index + 1].cp : 0;

    int   x0, y0, x1, y1;
    float x_shift = xpos - (float)floor(xpos); // subpixel shift along x
    float y_shift = ypos - (float)floor(ypos); // subpixel shift along y

    stbtt_GetCodepointHMetrics(font,
                               cp,
                               &advx, // advance from base x pos to next base pos
                               &lsb); // left side bearing

    // increase xpos with left side bearing if first character in line
    if (xpos == 0 && lsb < 0)
    {
      xpos    = (float)-lsb * scale;
      x_shift = xpos - (float)floor(xpos); // subpixel shift
    }

    stbtt_GetCodepointBitmapBoxSubpixel(font,
                                        cp,
                                        scale,
                                        scale,
                                        x_shift, // x axis subpixel shift
                                        y_shift, // y axis subpixel shift
                                        &x0,     // left edge of the glyph from origin
                                        &y0,     // top edge of the glyph from origin
                                        &x1,     // right edge of the glyph from origin
                                        &y1);    // bottom edge of the glyph from origin

    int w = x1 - x0;
    int h = y1 - y0;

    int size = w * h;

    // increase glyph baking bitmap size if needed
    if (size > txt.gcount)
    {
      txt.gcount = size;
      txt.gbytes = realloc(txt.gbytes, txt.gcount);
    }

    glyph.x       = xpos + x0;
    glyph.y       = ypos + y0;
    glyph.w       = w;
    glyph.h       = h;
    glyph.x_scale = scale;
    glyph.y_scale = scale;
    glyph.x_shift = x_shift;
    glyph.y_shift = y_shift;
    glyph.base_y  = ypos;
    glyph.asc     = (float)asc * scale;
    glyph.desc    = (float)desc * scale;
    glyph.cp      = cp;

    // advance x axis
    xpos += (advx * scale);

    // in case of space/invisible, set width based on pos
    if (w == 0) glyph.w = xpos - glyph.x;

    // advance with kerning
    if (ncp > 0) xpos += scale * stbtt_GetCodepointKernAdvance(font, cp, ncp);

    // line break
    if (cp == '\n' || cp == '\r') glyph.w = 0; // they should be invisible altough they get an empty unicode font face
    if (cp == ' ') spc_a = index;

    // store glyph
    glyphs[index] = glyph;

    if (style.multiline)
    {
      if (cp == '\n' || cp == '\r' || xpos > wth)
      {
        if (xpos > wth)
        {
          // check if we already fell back to this index, break word if yes
          if (spc_a == spc_p)
            index -= 1;
          else
            index = spc_a;
          spc_p = spc_a;
        }
        xpos = 0.0;
        ypos += (float)lineh * scale;
      }
    }
  }

  *nwth = xpos;
  *nhth = ypos;
}

void text_align_glyphs(glyph_t*    glyphs,
                       int         count,
                       textstyle_t style,
                       int         w,
                       int         h)
{
  if (count > 0)
  {
    // calculate vertical shift
    glyph_t head   = glyphs[0];
    glyph_t tail   = glyphs[count - 1];
    float   height = (tail.base_y - tail.desc) - (head.base_y - head.asc);
    float   vs     = 1.0; // vertical shift

    if (style.valign == VA_CENTER) vs = (h - height) / 2.0;
    if (style.valign == VA_BOTTOM) vs = h - height;

    vs = roundf(vs) + 1.0; // TODO investigate this a little, maybe no magic numbers are needed

    for (int i = 0; i < count; i++)
    {
      glyph_t g = glyphs[i];
      float   x = g.x;

      // get last glyph in row for row width

      float ex = x; // end x

      int ri; // row index
      int sc; // space count

      for (ri = i; ri < count; ri++)
      {
        glyph_t rg = glyphs[ri];          // row glyph
        if (rg.base_y != g.base_y) break; // last glyph in row
        ex = rg.x + rg.w;
        if (rg.cp == ' ') sc += 1; // count spaces
      }

      float rw = ex - x; // row width

      // calculate horizontal shift

      float hs = 0; // horizontal space before first glyph

      if (style.align == TA_RIGHT) hs = (float)w - rw;
      if (style.align == TA_CENTER) hs = ((float)w - rw) / 2.0; // space
      if (style.align == TA_JUSTIFY) hs = ((float)w - rw) / sc; // space

      // shift glyphs in both direction

      for (int ni = i; ni < ri; ni++)
      {
        glyphs[ni].x += (int)hs;
        glyphs[ni].y += (int)vs;
        glyphs[ni].base_y += (int)vs;
      }

      // jump to next row

      i = ri - 1;
    }
  }
}

void text_shift_glyphs(glyph_t*    glyphs,
                       int         count,
                       textstyle_t style)
{
  int x = style.margin_left;
  int y = style.margin_top;
  for (int i = 0; i < count; i++)
  {
    glyphs[i].x += x;
    glyphs[i].y += y;
    glyphs[i].base_y += y;
  }
}

void text_render_glyph(glyph_t g, textstyle_t style, bm_t* bitmap)
{
  if (g.w > 0 && g.h > 0)
  {
    gfx_rect(bitmap, 0, 0, bitmap->w, bitmap->h, style.backcolor, 0);

    // get or load font
    stbtt_fontinfo* font = MGET(txt.fonts, style.font);
    if (font == NULL)
    {
      text_font_load(style.font);
      font = MGET(txt.fonts, style.font);
      if (!font) return;
    }

    int size = g.w * g.h;

    // increase glyph baking bitmap size if needed
    if (size > txt.gcount)
    {
      txt.gcount = size;
      txt.gbytes = realloc(txt.gbytes, txt.gcount);
    }

    // bake
    stbtt_MakeCodepointBitmapSubpixel(font,
                                      txt.gbytes,
                                      g.w,       // out widht
                                      g.h,       // out height
                                      g.w,       // out stride
                                      g.x_scale, // scale x
                                      g.y_scale, // scale y
                                      g.x_shift, // shift x
                                      g.y_shift, // shift y
                                      g.cp);

    // insert to bitmap
    gfx_blend_8(bitmap,
                0,
                0,
                style.textcolor,
                txt.gbytes,
                g.w,
                g.h);
  }
}

void text_render_glyphs(glyph_t*    glyphs,
                        int         count,
                        textstyle_t style,
                        bm_t*       bitmap)
{
  gfx_rect(bitmap, 0, 0, bitmap->w, bitmap->h, style.backcolor, 0);

  // get or load font
  stbtt_fontinfo* font = MGET(txt.fonts, style.font);
  if (font == NULL)
  {
    text_font_load(style.font);
    font = MGET(txt.fonts, style.font);
    if (!font) return;
  }

  // draw glyphs
  for (int i = 0; i < count; i++)
  {
    glyph_t g = glyphs[i];

    // don't write bitmap in case of empty glyphs ( space )
    if (g.w > 0 && g.h > 0)
    {
      int size = g.w * g.h;

      // increase glyph baking bitmap size if needed
      if (size > txt.gcount)
      {
        txt.gcount = size;
        txt.gbytes = realloc(txt.gbytes, txt.gcount);
      }

      stbtt_MakeCodepointBitmapSubpixel(font,
                                        txt.gbytes,
                                        g.w,       // out widht
                                        g.h,       // out height
                                        g.w,       // out stride
                                        g.x_scale, // scale x
                                        g.y_scale, // scale y
                                        g.x_shift, // shift x
                                        g.y_shift, // shift y
                                        g.cp);

      gfx_blend_8(bitmap,
                  g.x,
                  g.y,
                  style.textcolor,
                  txt.gbytes,
                  g.w,
                  g.h);
    }
  }
}

void text_describe_glyphs(glyph_t* glyphs, int count)
{
  for (int i = 0; i < count; i++)
  {
    glyph_t g = glyphs[i];
    printf("%i cp %i xy %i %i wh %i %i\n", i, g.cp, g.x, g.y, g.w, g.h);
  }
}

void text_layout(glyph_t*    glyphs,
                 int         count,
                 textstyle_t style,
                 int         wth,
                 int         hth,
                 int*        nwth,
                 int*        nhth)
{
  if (style.margin_left == 0 && style.margin > 0) style.margin_left = style.margin;
  if (style.margin_right == 0 && style.margin > 0) style.margin_right = style.margin;
  if (style.margin_top == 0 && style.margin > 0) style.margin_top = style.margin;
  if (style.margin_bottom == 0 && style.margin > 0) style.margin_bottom = style.margin;

  int w = wth - style.margin_right - style.margin_left;
  int h = hth - style.margin_top - style.margin_bottom;

  text_break_glyphs(glyphs, count, style, w, h, nwth, nhth);
  text_align_glyphs(glyphs, count, style, w, h);
  text_shift_glyphs(glyphs, count, style);
}

void text_render(
    str_t*      text,
    textstyle_t style,
    bm_t*       bitmap)
{
  glyph_t* glyphs = malloc(sizeof(glyph_t) * text->length); // REL 0
  for (int i = 0; i < text->length; i++) glyphs[i].cp = text->codepoints[i];

  int nw;
  int nh;

  text_layout(glyphs, text->length, style, bitmap->w, bitmap->h, &nw, &nh);
  text_render_glyphs(glyphs, text->length, style, bitmap);

  free(glyphs); // REL 1
}

void text_measure(str_t* text, textstyle_t style, int w, int h, int* nw, int* nh)
{
  glyph_t* glyphs = malloc(sizeof(glyph_t) * text->length); // REL 0
  for (int i = 0; i < text->length; i++) glyphs[i].cp = text->codepoints[i];

  text_break_glyphs(glyphs, text->length, style, w, h, nw, nh);

  free(glyphs); // REL 1
}

#endif
