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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CFG_PATH_LOC "~/.config/i3-overview/config"
#define CFG_PATH_GLO "/usr/share/i3-overview/config"
#define WIN_CLASS "i3-overview"
#define WIN_TITLE "i3-overview"
#define KEY_META 133
#define GET_WORKSPACES_CMD "i3-msg -t get_workspaces"
#define GET_TREE_CMD "i3-msg -t get_tree"

int alive = 1;

void read_tree(vec_t* workspaces)
{
  char buff[100] = {0};

  FILE* pipe = popen(GET_WORKSPACES_CMD, "r"); // CLOSE 0

  char* wstree = cstr_new_cstring("{\"items\":"); // REL 0

  while (fgets(buff, sizeof(buff), pipe) != NULL) wstree = cstr_append(wstree, buff);

  wstree = cstr_append(wstree, "}");

  pclose(pipe); // CLOSE 0

  pipe = popen(GET_TREE_CMD, "r"); // CLOSE 0

  char* tree = cstr_new_cstring(""); // REL 0

  while (fgets(buff, sizeof(buff), pipe) != NULL) tree = cstr_append(tree, buff);

  pclose(pipe); // CLOSE 0

  tree_reader_extract(wstree, tree, workspaces);

  REL(wstree); // REL 1
  REL(tree);
}

void sighandler(int signal)
{
  alive = 0;
}

int main(int argc, char* argv[])
{
  printf("i3-overview v%i.%i by Milan Toth\n", VERSION, BUILD);

  /* close gracefully for SIGINT */

  if (signal(SIGINT, &sighandler) == SIG_ERR) printf("Could not set signal handler\n");

  /* parse parameters */

  char* cfg_path = NULL;

  const struct option long_options[] =
      {
          {"config", optional_argument, 0, 'c'},
          {0, 0, 0, 0},
      };

  int option       = 0;
  int option_index = 0;

  while ((option = getopt_long(argc, argv, "c", long_options, &option_index)) != -1)
  {
    if (option != '?') printf("parsing option %c value: %s\n", option, optarg);
    if (option == 'c') cfg_path = cstr_new_cstring(optarg); // REL 0
    if (option == '?') printf("-c --config= [path] \t use config file for session\n");
  }

  /* init config */

  config_init(); // DESTROY 0

  char* wrk_path     = cstr_new_path_normalize(argv[0], NULL);                                                                         // REL 1
  char* cfg_path_loc = cfg_path ? cstr_new_path_normalize(cfg_path, wrk_path) : cstr_new_path_normalize(CFG_PATH_LOC, getenv("HOME")); // REL 2
  char* cfg_path_glo = cstr_new_cstring(CFG_PATH_GLO);                                                                                 // REL 3

  printf("working path  : %s\n", wrk_path);
  printf("local config path   : %s\n", cfg_path_loc);
  printf("global config path   : %s\n", cfg_path_glo);

  if (config_read(cfg_path_loc) < 0)
  {
    if (config_read(cfg_path_glo) < 0)
      printf("no local or global config file found\n");
  }

  REL(cfg_path_glo); // REL 3
  REL(cfg_path_loc); // REL 2
  REL(wrk_path);     // REL 1

  /* init text rendeing */

  text_ft_init(); // DESTROY 1

  char* font_face = config_get("font_face");
  char* font_path = fontconfig_new_path(font_face ? font_face : ""); // REL 4

  config_set("font_path", font_path);

  REL(font_path); // REL 4

  /* init X11 */

  Display* display = XOpenDisplay(NULL);

  int opcode;
  int event;
  int error;

  if (!XQueryExtension(display, "XInputExtension", &opcode, &event, &error)) printf("X Input extension not available.\n");

  /* create overlay window */

  Window view_win = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 200, 100, 0, 0xFFFFFF, 0); // DESTROY 2

  XClassHint hint;

  hint.res_name  = WIN_CLASS;
  hint.res_class = WIN_CLASS;

  XSetClassHint(display, view_win, &hint);
  XStoreName(display, view_win, WIN_TITLE);

  XSelectInput(display, view_win, StructureNotifyMask);

  /* start listening for global key events */

  Window root_win = DefaultRootWindow(display);

  XIEventMask mask;
  mask.deviceid = XIAllDevices;
  mask.mask_len = XIMaskLen(XI_LASTEVENT);
  mask.mask     = calloc(mask.mask_len, sizeof(char)); // FREE 0

  XISetMask(mask.mask, XI_KeyPress);
  XISetMask(mask.mask, XI_KeyRelease);

  XISelectEvents(display, root_win, &mask, 1);
  XSync(display, False);

  free(mask.mask); // FREE 0

  int meta_pressed  = 0;
  int window_mapped = 0;

  bm_t* bitmap = NULL;

  while (alive)
  {
    XEvent               ev;
    XGenericEventCookie* cookie = (XGenericEventCookie*)&ev.xcookie;

    XNextEvent(display, (XEvent*)&ev); // FREE 1

    if (XGetEventData(display, cookie) && cookie->type == GenericEvent && cookie->extension == opcode)
    {
      XIDeviceEvent* event = cookie->data;

      switch (event->evtype)
      {
      case XI_KeyPress:
        if (event->detail == KEY_META || meta_pressed)
        {
          if (event->detail == KEY_META) meta_pressed = 1;

          vec_t* workspaces = VNEW(); // RET 0

          read_tree(workspaces);

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

              int snum   = DefaultScreen(display);
              int width  = DisplayWidth(display, snum);
              int height = DisplayHeight(display, snum);

              XMoveWindow(display, view_win, width / 2 - lay_wth / 2, height / 2 - lay_hth / 2);
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

            tree_drawer_draw(bitmap, workspaces);

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

    XFreeEventData(display, cookie); // FREE 1
  }

  XDestroyWindow(display, view_win); // DESTROY 2
  XSync(display, False);
  XCloseDisplay(display);

  /* cleanup */

  config_destroy();  // DESTROY 0
  text_ft_destroy(); // DESTROY 1

  if (bitmap) REL(bitmap);

  if (cfg_path) REL(cfg_path); // REL 0

#ifdef DEBUG
  mem_stats();
#endif
}
