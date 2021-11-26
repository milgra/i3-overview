#include "config.c"
#include "fontconfig.c"
#include "kvlines.c"
#include "text_ft.c"
#include "tree_drawer.c"
#include "tree_reader.c"
#include "zc_bitmap.c"
#include "zc_cstring.c"
#include "zc_cstrpath.c"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CFG_PATH_LOC "~/.config/i3-overview/config"
#define CFG_PATH_GLO "/usr/share/i3-overview/config"

struct
{
  char* cfg_par; // config path parameter
  char* ws_json_par;
  char* tree_json_par;
  char* output_par;
} zm = {0};

int alive = 1;

int main(int argc, char* argv[])
{
  printf("i3-overview v%i.%i by Milan Toth\n", VERSION, BUILD);

  const struct option long_options[] =
      {
          {"config", optional_argument, 0, 'c'},
          {"ws_json", optional_argument, 0, 'w'},
          {"tree_json", optional_argument, 0, 't'},
          {"output", optional_argument, 0, 'o'},
          {0, 0, 0, 0},
      };

  int option       = 0;
  int option_index = 0;

  while ((option = getopt_long(argc, argv, "c:w:t:o:", long_options, &option_index)) != -1)
  {
    if (option != '?') printf("parsing option %c value: %s\n", option, optarg);
    if (option == 'c') zm.cfg_par = cstr_new_cstring(optarg);       // REL 0
    if (option == 'w') zm.ws_json_par = cstr_new_cstring(optarg);   // REL 0
    if (option == 't') zm.tree_json_par = cstr_new_cstring(optarg); // REL 0
    if (option == 'o') zm.output_par = cstr_new_cstring(optarg);    // REL 0
    if (option == '?')
    {
      printf("-c --config= [path] \t use config file for session\n");
      printf("-w --ws_json= [path] \t use workspaces source file\n");
      printf("-t --tree_json= [path] \t use tree source file\n");
      printf("-o --output= [path] \t render to file \n");
    }
  }

  bm_t* bitmap = NULL;

  if (zm.output_par == NULL)
  {
    srand((unsigned int)time(NULL));

    config_init(); // destroy 1
    text_ft_init();

    char* path = argv[0];

    char* wrk_path     = cstr_new_path_normalize(path, NULL);                                                                                // REL 0
    char* cfg_path_loc = zm.cfg_par ? cstr_new_path_normalize(zm.cfg_par, wrk_path) : cstr_new_path_normalize(CFG_PATH_LOC, getenv("HOME")); // REL 1
    char* cfg_path_glo = cstr_new_cstring(CFG_PATH_GLO);                                                                                     // REL 2

    // print path info to console

    printf("working path  : %s\n", wrk_path);
    printf("local config path   : %s\n", cfg_path_loc);
    printf("global config path   : %s\n", cfg_path_glo);

    // read config, it overwrites defaults if exists

    if (config_read(cfg_path_loc) < 0)
    {
      if (config_read(cfg_path_glo) < 0)
        printf("no local or global config file found\n");
      else
        printf("using global config\n");
    }
    else
      printf("using local config\n");

    // init font

    char* font_face = config_get("font_face");
    char* font_path = fontconfig_new_path(font_face ? font_face : ""); // REL 3

    config_set("font_path", font_path);

    // init X11

    int opcode;
    int event;
    int error;

    Display* display = XOpenDisplay(NULL);

    if (!XQueryExtension(display, "XInputExtension", &opcode, &event, &error))
    {
      printf("X Input extension not available.\n");
    }

    int blackColor = BlackPixel(display, DefaultScreen(display));
    int whiteColor = WhitePixel(display, DefaultScreen(display));

    // Create the window

    Window view_win = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 200, 100, 0, whiteColor, blackColor);

    XClassHint hint;

    hint.res_name  = "i3-overview";
    hint.res_class = "i3-overview";

    XSetClassHint(display, view_win, &hint);
    XStoreName(display, view_win, "i3-overview");

    // We want to get MapNotify events

    XSelectInput(display, view_win, StructureNotifyMask);

    // Get root window

    Window root_win = DefaultRootWindow(display);

    // Get key events

    XIEventMask mask;
    mask.deviceid = XIAllDevices;
    mask.mask_len = XIMaskLen(XI_LASTEVENT);
    mask.mask     = calloc(mask.mask_len, sizeof(char));

    XISetMask(mask.mask, XI_KeyPress);
    XISetMask(mask.mask, XI_KeyRelease);

    XISelectEvents(display, root_win, &mask, 1);
    XSync(display, False);

    free(mask.mask);

    int meta_pressed  = 0;
    int window_mapped = 0;

    // Event loop

    while (alive)
    {
      XEvent               ev;
      XGenericEventCookie* cookie = (XGenericEventCookie*)&ev.xcookie;

      XNextEvent(display, (XEvent*)&ev);

      if (XGetEventData(display, cookie) &&
          cookie->type == GenericEvent &&
          cookie->extension == opcode)
      {

        XIDeviceEvent* event = cookie->data;

        /* printf("    device: %d (%d)\n", event->deviceid, event->sourceid); */
        /* printf("    detail: %d\n", event->detail); */

        switch (event->evtype)
        {
        case XI_KeyPress:
          if (event->detail == 9) alive = 0;
          if (event->detail == 133 || meta_pressed)
          {
            if (event->detail == 133) meta_pressed = 1;

            char buff[100] = {0};

            // get i3 workspaces

            FILE* pipe   = popen("i3-msg -t get_workspaces", "r"); // CLOSE 0
            char* wstree = cstr_new_cstring("{\"items\":");        // REL 0

            while (fgets(buff, sizeof(buff), pipe) != NULL) wstree = cstr_append(wstree, buff);

            wstree = cstr_append(wstree, "}");
            pclose(pipe); // CLOSE 0

            pipe       = popen("i3-msg -t get_tree", "r"); // CLOSE 0
            char* tree = cstr_new_cstring("");             // REL 0

            while (fgets(buff, sizeof(buff), pipe) != NULL) tree = cstr_append(tree, buff);

            pclose(pipe); // CLOSE 0

            vec_t* workspaces = VNEW(); // RET 0

            analyzer_extract(wstree, tree, workspaces);

            REL(wstree); // REL 1
            REL(tree);

            i3_workspace_t* ws  = workspaces->data[0];
            i3_workspace_t* wsl = workspaces->data[workspaces->length - 1];

            if (ws->width > 0 && ws->height > 0)
            {
              int gap  = 25;
              int cols = 5;
              int rows = (int)ceilf((float)wsl->number / 5.0);

              int lay_wth = cols * (ws->width / 8) + (cols + 1) * gap;
              int lay_hth = rows * (ws->height / 8) + (rows + 1) * gap;

              // resize window if necessary

              XWindowAttributes gwa;
              XGetWindowAttributes(display, view_win, &gwa);

              int wd1 = gwa.width;
              int ht1 = gwa.height;

              if (wd1 != lay_wth || ht1 != lay_hth)
              {
                XResizeWindow(display, view_win, lay_wth, lay_hth);

                int snum   = DefaultScreen(display);
                int width  = DisplayWidth(display, snum);
                int height = DisplayHeight(display, snum);

                XMoveWindow(display, view_win, width / 2 - lay_wth / 2, height / 2 - lay_hth / 2);
              }

              // map window if necessary

              if (!window_mapped)
              {
                XMapWindow(display, view_win);

                for (;;)
                {
                  XEvent e;
                  XNextEvent(display, &e);
                  if (e.type == MapNotify)
                    break;
                }
                window_mapped = 1;
              }

              XGetWindowAttributes(display, view_win, &gwa);

              wd1 = gwa.width;
              ht1 = gwa.height;

              // create texture bitmap

              if (bitmap == NULL || bitmap->w != lay_wth || bitmap->h != lay_hth)
              {
                if (bitmap != NULL) REL(bitmap);
                bitmap = bm_new(lay_wth, lay_hth); // REL 0
              }

              renderer_draw(bitmap, workspaces);

              XImage* image = XGetImage(display, view_win, 0, 0, lay_wth, lay_hth, AllPlanes, ZPixmap);

              for (int y = 0; y < lay_hth; y++)
              {
                for (int x = 0; x < lay_wth; x++)
                {
                  int      index = y * lay_wth * 4 + x * 4;
                  uint8_t  r     = bitmap->data[index];
                  uint8_t  g     = bitmap->data[index + 1];
                  uint8_t  b     = bitmap->data[index + 2];
                  uint32_t pixel = (r << 16) | (g << 8) | b;
                  XPutPixel(image, x, y, pixel);
                }
              }

              GC gc = XCreateGC(display, view_win, 0, NULL);
              XPutImage(display, view_win, gc, image, 0, 0, 0, 0, lay_wth, lay_hth);
              XFreeGC(display, gc);

              XDestroyImage(image);
            }

            REL(workspaces);
          }
          break;
        case XI_KeyRelease:
          if (event->detail == 133)
          {
            XUnmapWindow(display, view_win);

            meta_pressed  = 0;
            window_mapped = 0;
          }
          break;
        }
      }

      XFreeEventData(display, cookie);
    }

    XDestroyWindow(display, root_win);
    XSync(display, False);
    XCloseDisplay(display);

    // cleanup

    REL(wrk_path);     // REL 0
    REL(cfg_path_loc); // REL 1
    REL(cfg_path_glo); // REL 2
    REL(font_path);    // REL 3

    config_destroy(); // destroy 1
    text_ft_destroy();
  }
  else
  {
    // just render source file and exit

    char*  ws_json    = cstr_new_file(zm.ws_json_par);
    char*  tree_json  = cstr_new_file(zm.tree_json_par);
    vec_t* workspaces = VNEW(); // RET 0

    analyzer_extract(ws_json, tree_json, workspaces);

    // get needed bitmap size

    i3_workspace_t* ws  = workspaces->data[0];
    i3_workspace_t* wsl = workspaces->data[workspaces->length - 1];

    if (ws->width > 0 && ws->height > 0)
    {
      int gap  = 25;
      int cols = 5;
      int rows = (int)ceilf((float)wsl->number / 5.0);

      int lay_wth = cols * (ws->width / 8) + (cols + 1) * gap;
      int lay_hth = rows * (ws->height / 8) + (rows + 1) * gap;

      // create texture bitmap

      if (bitmap == NULL || bitmap->w != lay_wth || bitmap->h != lay_hth)
      {
        if (bitmap) REL(bitmap);
        bitmap = bm_new(lay_wth, lay_hth); // REL 0
      }

      renderer_draw(bitmap, workspaces);

      // save gitmap to output file

      REL(ws_json);
      REL(tree_json);
      REL(workspaces);
    }
  }

  if (bitmap) REL(bitmap);

  if (zm.cfg_par) REL(zm.cfg_par);             // REL 0
  if (zm.ws_json_par) REL(zm.ws_json_par);     // REL 0
  if (zm.tree_json_par) REL(zm.tree_json_par); // REL 0
  if (zm.output_par) REL(zm.output_par);       // REL 0

#ifdef DEBUG
  mem_stats();
#endif
}
