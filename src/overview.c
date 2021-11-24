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
#include <gtk/gtk.h>
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
  char  showpanel;
  char  meta_state;
  char  panel_visible;
} zm = {0};

GtkWidget* window = NULL;
GtkWidget* image  = NULL;
bm_t*      bm     = NULL;

int f_show  = 0;
int f_hide  = 0;
int visible = 0;
int resize  = 0;

gboolean redraw(gpointer data)
{
  if (f_show)
  {
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

      // create texture bitmap

      if (bm == NULL || bm->w != lay_wth || bm->h != lay_hth)
      {
        bm     = bm_new(lay_wth, lay_hth); // REL 0
        resize = 1;
      }

      renderer_draw(bm, workspaces);

      GdkPixbuf* pb = gdk_pixbuf_new_from_data(bm->data, GDK_COLORSPACE_RGB, TRUE, 8, bm->w, bm->h, bm->w * 4, NULL, NULL);

      gtk_image_set_from_pixbuf((GtkImage*)image, pb);

      g_object_unref(pb);
    }

    REL(workspaces);

    if (!visible)
    {
      /* if (resize) */
      /* { */
      /*   resize = 0; */
      /* } */
      gtk_widget_show(window);

      gint rwidth, rheight;

      // get screen geom

      GdkWindow* root = gtk_widget_get_root_window(GTK_WIDGET(window));

      gdk_window_get_geometry(root, NULL, NULL, &rwidth, &rheight);

      gtk_window_resize(GTK_WINDOW(window), bm->w, bm->h);

      gtk_window_move(GTK_WINDOW(window), (rwidth - bm->w) / 2, (rheight - bm->h) / 2);

      visible = 1;
    }
    f_show = 0;
  }

  if (f_hide)
  {
    if (visible)
    {
      gtk_widget_hide(window);
      visible = 0;
    }
    f_hide = 0;
  }

  return TRUE;
}

void show()
{
  f_show = 1;
  redraw(NULL);
}

void hide()
{
  f_hide = 1;
  redraw(NULL);
}

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

  if (zm.output_par == NULL)
  {
    // wm_init(init, update, render, destroy, NULL, 1);
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

      if (bm == NULL || bm->w != lay_wth || bm->h != lay_hth)
      {
        bm     = bm_new(lay_wth, lay_hth); // REL 0
        resize = 1;
      }

      renderer_draw(bm, workspaces);

      // save gitmap to output file

      REL(ws_json);
      REL(tree_json);
      REL(workspaces);
    }
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

  char* font_face = config_get("font_face");
  char* font_path = fontconfig_new_path(font_face ? font_face : ""); // REL 3

  printf("font_path %s\n", font_path);

  config_set("font_path", font_path);

  // main loop

  // Read a raw image data from the disk and put it in the buffer.
  // ....

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  image  = gtk_image_new();

  listener_start(show, hide); // destroy 2

  gtk_window_set_title(GTK_WINDOW(window), "Image Viewer");

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_container_add(GTK_CONTAINER(window), image);

  gtk_widget_show_all(window);
  gtk_widget_hide(window);

  gtk_main();

  g_object_unref(window);
  g_object_unref(image);

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
