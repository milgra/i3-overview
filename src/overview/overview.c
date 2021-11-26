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
#define GAP 25
#define COLS 5

int alive = 1;

void read_tree(vec_t* workspaces)
{
  char  buff[100];
  char* ws_json   = NULL; // REL 0
  char* tree_json = NULL; // REL 1

  FILE* pipe = popen(GET_WORKSPACES_CMD, "r"); // CLOSE 0
  ws_json    = cstr_new_cstring("{\"items\":");
  while (fgets(buff, sizeof(buff), pipe) != NULL) ws_json = cstr_append(ws_json, buff);
  ws_json = cstr_append(ws_json, "}");
  pclose(pipe); // CLOSE 0

  pipe      = popen(GET_TREE_CMD, "r"); // CLOSE 0
  tree_json = cstr_new_cstring("");
  while (fgets(buff, sizeof(buff), pipe) != NULL) tree_json = cstr_append(tree_json, buff);
  pclose(pipe); // CLOSE 0

  tree_reader_extract(ws_json, tree_json, workspaces);

  REL(ws_json);   // REL 0
  REL(tree_json); // REL 1
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

  bm_t* bitmap = bm_new(1, 1); // REL 5

  while (alive)
  {
    XEvent               ev;
    XGenericEventCookie* cookie = (XGenericEventCookie*)&ev.xcookie;

    XNextEvent(display, (XEvent*)&ev); // FREE 1

    if (XGetEventData(display, cookie) && cookie->type == GenericEvent && cookie->extension == opcode)
    {
      XIDeviceEvent* event = cookie->data;

      if (event->evtype == XI_KeyPress)
      {
        if (event->detail == KEY_META || meta_pressed)
        {
          if (event->detail == KEY_META) meta_pressed = 1;

          vec_t* workspaces = VNEW(); // REL 6

          read_tree(workspaces);

          i3_workspace_t* ws  = workspaces->data[0];
          i3_workspace_t* wsl = workspaces->data[workspaces->length - 1];

          if (ws->width > 0 && ws->height > 0)
          {
            int gap  = GAP;
            int cols = COLS;
            int rows = (int)ceilf((float)wsl->number / COLS);

            int lay_wth = cols * (ws->width / 8) + (cols + 1) * gap;
            int lay_hth = rows * (ws->height / 8) + (rows + 1) * gap;

            XWindowAttributes win_attr;
            XGetWindowAttributes(display, view_win, &win_attr);

            int win_wth = win_attr.width;
            int win_hth = win_attr.height;

            int screen  = DefaultScreen(display);
            int scr_wth = DisplayWidth(display, screen);
            int scr_hth = DisplayHeight(display, screen);

            /* resize if needed */

            if (win_wth != lay_wth || win_hth != lay_hth)
            {
              XResizeWindow(display, view_win, lay_wth, lay_hth);
              XMoveWindow(display, view_win, scr_wth / 2 - lay_wth / 2, scr_hth / 2 - lay_hth / 2);
            }
            /* map if necessary */

            if (!window_mapped)
            {
              window_mapped = 1;
              XMapWindow(display, view_win);
              XMoveWindow(display, view_win, scr_wth / 2 - lay_wth / 2, scr_hth / 2 - lay_hth / 2);
            }

            /* flush move, resize and map commands */

            XSync(display, False);

            /* create overlay bitmap */

            if (bitmap->w != lay_wth || bitmap->h != lay_hth)
            {
              REL(bitmap);
              bitmap = bm_new(lay_wth, lay_hth); // REL 5
            }

            tree_drawer_draw(bitmap, workspaces, GAP, COLS, 8.0, config_get("font_path"));

            XImage* image = XGetImage(display, view_win, 0, 0, lay_wth, lay_hth, AllPlanes, ZPixmap); // DESTROY 3

            uint8_t* data = bitmap->data;

            for (int y = 0; y < lay_hth; y++)
            {
              for (int x = 0; x < lay_wth; x++)
              {
                uint8_t  r     = data[0];
                uint8_t  g     = data[1];
                uint8_t  b     = data[2];
                uint32_t pixel = (r << 16) | (g << 8) | b;

                XPutPixel(image, x, y, pixel);

                data += 4;
              }
            }

            GC gc = XCreateGC(display, view_win, 0, NULL); // FREE 0
            XPutImage(display, view_win, gc, image, 0, 0, 0, 0, lay_wth, lay_hth);

            /* cleanup */

            XFreeGC(display, gc); // FREE 0
            XDestroyImage(image); // DESTROY 3
          }

          REL(workspaces); // REL 6
        }
      }
      else if (event->evtype == XI_KeyRelease)
      {
        if (event->detail == KEY_META)
        {
          XUnmapWindow(display, view_win);
          XSync(display, False);

          meta_pressed  = 0;
          window_mapped = 0;
        }
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

  REL(bitmap); // REL 5

  if (cfg_path) REL(cfg_path); // REL 0

#ifdef DEBUG
  mem_stats();
#endif
}
