#ifndef tree_drawer_h
#define tree_drawer_h

#include "zc_bitmap.c"
#include "zc_vector.c"

void tree_drawer_draw(bm_t*    bm,
                      vec_t*   workspaces,
                      int      gap,
                      int      cols,
                      float    scale,
                      char*    font_path,
                      uint32_t text_main_color,
                      uint32_t text_sub_color);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "text.c"
#include "tree_reader.c"
#include "zc_graphics.c"

void tree_drawer_draw(bm_t*    bm,
                      vec_t*   workspaces,
                      int      gap,
                      int      cols,
                      float    scale,
                      char*    font_path,
                      uint32_t text_main_color,
                      uint32_t text_sub_color)
{
  i3_workspace_t* ws0 = workspaces->data[0];
  i3_workspace_t* wsl = workspaces->data[workspaces->length - 1];

  int max = ceilf((float)wsl->number / cols) * cols;

  int wsw = ws0->width / scale;
  int wsh = ws0->height / scale;

  /* draw workspace backgrounds including empty */

  for (int wsi = 0; wsi < max; wsi++)
  {
    int cx = gap + wsi % cols * (wsw + gap);
    int cy = gap + wsi / cols * (wsh + gap);

    gfx_rect(bm, cx, cy, wsw, wsh, 0x112255FF, 0);
    gfx_rect(bm, cx + 1, cy + 1, wsw - 2, wsh - 2, 0x000000FF, 0);
  }

  for (int wsi = 0; wsi < workspaces->length; wsi++)
  {
    i3_workspace_t* ws = workspaces->data[wsi];

    int num = ws->number;

    int cx = gap + (num - 1) % cols * (wsw + gap);
    int cy = gap + (num - 1) / cols * (wsh + gap);

    /* draw focused workspace background */

    if (ws->focused) gfx_rect(bm, cx + 1, cy + 1, wsw - 2, wsh - 2, 0x222255FF, 0);

    /* draw windows */

    for (int wii = 0; wii < ws->windows->length; wii++)
    {
      i3_window_t* wi = ws->windows->data[wii];

      if (strstr(wi->class, "overview") == NULL)
      {
        int wiw = roundf((float)wi->width / scale);
        int wih = roundf((float)wi->height / scale);
        int wix = roundf(((float)wi->x - (float)ws->x) / scale);
        int wiy = roundf(((float)wi->y - (float)ws->y) / scale);

        int wcx = cx + wix;
        int wcy = cy + wiy;

        textstyle_t ts = {0};
        ts.font        = font_path;
        ts.margin      = 5;
        ts.margin_top  = -7;
        ts.align       = TA_LEFT;
        ts.valign      = VA_TOP;
        ts.size        = 14.0;
        ts.textcolor   = text_main_color;
        ts.backcolor   = 0x0000022FF;
        ts.multiline   = 0;

        /* highlight focused workspaces's windows */

        if (ws->focused)
        {
          ts.textcolor = 0xFFFFFFFF;
          ts.backcolor = 0x222255FF;
        }

        /* draw class */

        bm_t* tbm = bm_new(wiw - 4, wih - 4); // REL 0

        str_t* str = str_new(); // REL 1

        str_add_bytearray(str, wi->class);

        int grey = 0xFF - rand() % 0x55;

        text_render(str, ts, tbm);

        str_reset(str);

        /* draw title */

        str_add_bytearray(str, wi->title);

        ts.margin_top  = 10;
        ts.size        = 12.0;
        ts.textcolor   = text_sub_color;
        ts.backcolor   = 0;
        ts.line_height = 12;
        ts.multiline   = 1;

        text_render(str, ts, tbm);

        /* draw frame */

        gfx_rect(bm, wcx + 1, wcy + 1, wiw - 2, wih - 2, 0xAADDFFFF, 0);

        if (ws->focused) gfx_rect(bm, wcx + 1, wcy + 1, wiw - 2, wih - 2, 0xFFFFFFFF, 0);

        /* insert text bitmap */

        gfx_insert(bm, tbm, wcx + 2, wcy + 2);

        REL(str); // REL 1
        REL(tbm); // REL 0
      }
    }
  }

  /* draw all workspace numbers */

  for (int wsi = 0; wsi < max; wsi++)
  {
    int cx = gap + wsi % cols * (wsw + gap);
    int cy = gap + wsi / cols * (wsh + gap);

    textstyle_t its = {0};
    its.font        = font_path;
    its.align       = TA_RIGHT;
    its.valign      = VA_TOP;
    its.size        = 20.0;
    its.textcolor   = 0xFFFFFFFF;
    its.backcolor   = 0x00002200;

    bm_t*  tbm     = bm_new(wsw, wsh); // REL 0
    str_t* str     = str_new();        // REL 1
    char   nums[4] = {0};

    snprintf(nums, 4, "%i", wsi + 1);
    str_add_bytearray(str, nums);

    text_render(str, its, tbm);
    gfx_blend_bitmap(bm, tbm, cx + 4, cy - 22);

    REL(str); // REL 1
    REL(tbm); // REL 0
  }
}

#endif
