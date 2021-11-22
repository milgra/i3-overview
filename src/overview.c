#include "analyzer.c"
#include "config.c"
#include "fontconfig.c"
#include "kvlines.c"
#include "listener.c"
#include "renderer.c"
#include "text_ft.c"
#include "zc_bitmap.c"
#include "zc_cstring.c"
#include "zc_cstrpath.c"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CFG_PATH_LOC "~/.config/i3-overview/config"
#define CFG_PATH_GLO "/usr/share/i3-overview/config"

struct
{
  char* cfg_par; // config path parameter
  char* src_par; // render source parameter
  char* tgt_par; // render source parameter
  char  showpanel;
  char  meta_state;
  char  panel_visible;
} zm = {0};

int main(int argc, char* argv[])
{
  printf("i3-overview v%i.%i by Milan Toth\n", VERSION, BUILD);

  const struct option long_options[] =
      {
          {"config", optional_argument, 0, 'c'},
          {"source", optional_argument, 0, 's'},
          {"target", optional_argument, 0, 't'},
          {0, 0, 0, 0},
      };

  int option       = 0;
  int option_index = 0;

  while ((option = getopt_long(argc, argv, "r:c:s:", long_options, &option_index)) != -1)
  {
    if (option != '?') printf("parsing option %c value: %s\n", option, optarg);
    if (option == 'c') zm.cfg_par = cstr_new_cstring(optarg); // REL 0
    if (option == 's') zm.src_par = cstr_new_cstring(optarg); // REL 0
    if (option == 't') zm.tgt_par = cstr_new_cstring(optarg); // REL 0
    if (option == '?')
    {
      printf("-c --config= [path] \t use config file for session\n");
      printf("-s --source= [path] \t use tree source file\n");
      printf("-t --target= [path] \t render to file \n");
    }
  }

  if (zm.src_par == NULL)
  {
    // wm_init(init, update, render, destroy, NULL, 1);
  }
  else
  {
    // just render source file and exit
  }

  srand((unsigned int)time(NULL));

  XInitThreads();

  gtk_init(&argc, &argv);

  text_ft_init();

  config_init(); // destroy 1

  char* path = argv[0];

  printf("PATH %s\n", path);

  char* wrk_path     = cstr_new_path_normalize(path, NULL);                                                                                // REL 0
  char* cfg_path_loc = zm.cfg_par ? cstr_new_path_normalize(zm.cfg_par, wrk_path) : cstr_new_path_normalize(CFG_PATH_LOC, getenv("HOME")); // REL 1
  char* cfg_path_glo = cstr_new_cstring(CFG_PATH_GLO);                                                                                     // REL 2
  char* src_path     = zm.src_par ? cstr_new_path_normalize(zm.src_par, wrk_path) : NULL;                                                  // REL 1
  char* tgt_path     = zm.tgt_par ? cstr_new_path_normalize(zm.tgt_par, wrk_path) : NULL;                                                  // REL 1

  // print path info to console

  printf("working path  : %s\n", wrk_path);
  printf("local config path   : %s\n", cfg_path_loc);
  printf("global config path   : %s\n", cfg_path_glo);
  printf("source path   : %s\n", src_path);
  printf("target path   : %s\n", tgt_path);

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

  char* font_face = config_get("font_face");
  char* font_path = fontconfig_new_path(font_face ? font_face : ""); // REL 3

  printf("font_path %s\n", font_path);

  config_set("font_path", font_path);

  // main loop

  // Read a raw image data from the disk and put it in the buffer.
  // ....

  GtkWidget* window;
  GtkWidget* image;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  image  = gtk_image_new();

  listener_set_window(window, image);
  listener_start(); // destroy 2

  gtk_window_set_title(GTK_WINDOW(window), "Image Viewer");

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_container_add(GTK_CONTAINER(window), image);

  gtk_widget_show_all(window);
  gtk_widget_hide(window);

  gtk_main();

  // cleanup

  REL(wrk_path);     // REL 0
  REL(cfg_path_loc); // REL 1
  REL(cfg_path_glo); // REL 2
  REL(font_path);    // REL 3

  config_destroy(); // destroy 1

  if (zm.cfg_par) REL(zm.cfg_par); // REL 0

#ifdef DEBUG
  mem_stats();
#endif
}

bm_t* bm = NULL;

void tg_overview_gen()
{
  // analyzer workspaces

  vec_t* workspaces = VNEW(); // RET 0

  analyzer_get(workspaces);

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

    if (bm == NULL || bm->w != lay_wth || bm->h != lay_hth)
    {
      bm = bm_new(lay_wth, lay_hth); // REL 0
    }

    renderer_draw(bm, workspaces);
  }

  REL(workspaces);
}
