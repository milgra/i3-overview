#ifndef renderer_h
#define renderer_h

#include "zc_bitmap.c"
#include "zc_vector.c"

void renderer_draw(bm_t* bm, vec_t* workspaces);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "config.c"
#include "text.c"
#include "text_ft.c"
#include "tree_reader.c"
#include "zc_graphics.c"

void renderer_draw(bm_t* bm, vec_t* workspaces)
{
  gfx_rect(bm, 0, 0, bm->w, bm->h, 0x000000FF, 0);

  i3_workspace_t* ws0 = workspaces->data[0];
  i3_workspace_t* wsl = workspaces->data[workspaces->length - 1];

  int gap = 25;
  int max = ceilf((float)wsl->number / 5.0) * 5;

  int wsw = ws0->width / 8;
  int wsh = ws0->height / 8;

  // draw all workspaces including empty

  for (int wsi = 0; wsi < max; wsi++)
  {
    int cx = gap + wsi % 5 * (wsw + gap);
    int cy = gap + wsi / 5 * (wsh + gap);

    gfx_rect(bm, cx, cy, wsw, wsh, 0x112255FF, 0);
    gfx_rect(bm, cx + 1, cy + 1, wsw - 2, wsh - 2, 0x000000FF, 0);
  }

  // draw workspaces

  for (int wsi = 0; wsi < workspaces->length; wsi++)
  {
    i3_workspace_t* ws = workspaces->data[wsi];

    int num = ws->number;

    int cx = gap + (num - 1) % 5 * (wsw + gap);
    int cy = gap + (num - 1) / 5 * (wsh + gap);

    if (ws->focused)
    {
      gfx_rect(bm, cx + 1, cy + 1, wsw - 2, wsh - 2, 0x222255FF, 0);
    }

    // draw windows

    for (int wii = 0; wii < ws->windows->length; wii++)
    {
      i3_window_t* wi = ws->windows->data[wii];

      if (strstr(wi->class, "overview") == NULL)
      {
        // draw window

        int wiw = roundf((float)wi->width / 8.0);
        int wih = roundf((float)wi->height / 8.0);
        int wix = roundf(((float)wi->x - (float)ws->x) / 8.0);
        int wiy = roundf(((float)wi->y - (float)ws->y) / 8.0);

        // printf("wiw %i wih %i %i %i\n", wiw, wih, wi->width, wi->height);
        // printf("wix %i wiy %i %i %i\n", wix, wiy, wi->x, wi->y);

        int wcx = cx + wix;
        int wcy = cy + wiy;

        // draw class

        textstyle_t ts = {0};
        ts.font        = config_get("font_path");
        ts.margin      = 5;
        ts.margin_top  = -7;
        ts.align       = TA_LEFT;
        ts.valign      = VA_TOP;
        ts.size        = 14.0;
        ts.textcolor   = 0xFFFFFFFF;
        ts.backcolor   = 0x000022FF;
        ts.multiline   = 0;

        /* switch (wi->class[0]) */
        /* { */
        /* case 'G': */
        /*   ts.backcolor = 0x555500FF; */
        /*   break; */
        /* case 'U': */
        /*   ts.backcolor = 0x000000FF; */
        /*   break; */
        /* } */

        if (ws->focused)
        {
          ts.textcolor = 0xFFFFFFFF;
          ts.backcolor = 0x222255FF;
        }

        bm_t* tbm = bm_new(wiw - 4, wih - 4);

        str_t* str = str_new();
        str_add_bytearray(str, wi->class);

        int grey = 0xFF - rand() % 0x55;

        text_ft_render(str, ts, tbm);

        str_reset(str);
        str_add_bytearray(str, wi->title);

        // draw title

        ts.margin_top  = 10;
        ts.size        = 12.0;
        ts.textcolor   = 0xAADDFFFF;
        ts.backcolor   = 0;
        ts.line_height = 12;
        ts.multiline   = 1;

        text_ft_render(str, ts, tbm);

        REL(str);

        gfx_rect(bm, wcx + 1, wcy + 1, wiw - 2, wih - 2, 0xAADDFFFF, 0);

        if (ws->focused)
        {
          gfx_rect(bm, wcx + 1, wcy + 1, wiw - 2, wih - 2, 0xFFFFFFFF, 0);
        }

        gfx_insert(bm, tbm, wcx + 2, wcy + 2);

        REL(tbm);
      }
    }
  }

  // draw all workspace numbers

  for (int wsi = 0; wsi < max; wsi++)
  {
    int cx = gap + wsi % 5 * (wsw + gap);
    int cy = gap + wsi / 5 * (wsh + gap);

    textstyle_t its = {0};
    its.font        = config_get("font_path");
    its.align       = TA_RIGHT;
    its.valign      = VA_TOP;
    its.size        = 20.0;
    its.textcolor   = 0xFFFFFFFF;
    its.backcolor   = 0x00002200;

    bm_t*  tbm     = bm_new(wsw, wsh);
    str_t* str     = str_new(); // REL 0
    char   nums[4] = {0};

    snprintf(nums, 4, "%i", wsi + 1);
    str_add_bytearray(str, nums);

    text_ft_render(str, its, tbm);
    gfx_blend_bitmap(bm, tbm, cx + 4, cy - 22);

    REL(str);
    REL(tbm);
  }
}

#endif
