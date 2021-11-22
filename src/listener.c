#ifndef listener_h
#define listener_h
#include <gtk/gtk.h>

int  listener_get_meta_state();
int  listener_get_change_state();
void listener_start();
void listener_destroy();
void listener_set_window(GtkWidget* win, GtkWidget* img);

#endif

#if __INCLUDE_LEVEL__ == 0

#include "analyzer.c"
#include "renderer.c"
#include "zc_bitmap.c"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int listener_alive = 1;
int lmeta_pressed  = 0;
int lenter_pressed = 0;
int lesc_pressed   = 0;
int key_pressed    = 0;

GtkWidget* listener_window;
GtkWidget* l_image;

void listener_set_window(GtkWidget* win, GtkWidget* img)
{
  listener_window = win;
  l_image         = img;
}

int listener_get_meta_state()
{
  return lmeta_pressed;
}

int listener_get_change_state()
{
  int res     = key_pressed;
  key_pressed = 0;
  return res;
}

bm_t* l_bm = NULL;

static void print_deviceevent(XIDeviceEvent* event)
{
  double* val;
  int     i;

  printf("    device: %d (%d)\n", event->deviceid, event->sourceid);
  printf("    detail: %d\n", event->detail);
  switch (event->evtype)
  {
  case XI_KeyPress:
    printf("    flags: %s\n", (event->flags & XIKeyRepeat) ? "repeat" : "");
    if (event->detail == 133)
    {
      lmeta_pressed = 1;

      /* const int Width = 920, Height = 200; */
      /* char*     buffer = (char*)malloc(3 * Width * Height); */

      /* for (int y = 0; y < Height; y++) */
      /* { */
      /*   for (int x = 0; x < Width; x++) */
      /*   { */
      /*     int i         = (y * Width * 3) + x * 3; */
      /*     buffer[i]     = rand() % 255; */
      /*     buffer[i + 1] = rand() % 255; */
      /*     buffer[i + 2] = rand() % 255; */
      /*   } */
      /* } */

      /* GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(buffer, GDK_COLORSPACE_RGB, */
      /*                                              FALSE, 8, Width, Height, Width * 3, NULL, NULL); */

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

        if (l_bm == NULL || l_bm->w != lay_wth || l_bm->h != lay_hth)
        {
          l_bm = bm_new(lay_wth, lay_hth); // REL 0
        }

        renderer_draw(l_bm, workspaces);

        REL(workspaces);

        GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data(l_bm->data, GDK_COLORSPACE_RGB, TRUE, 8, l_bm->w, l_bm->h, l_bm->w * 4, NULL, NULL);

        gtk_image_set_from_pixbuf((GtkImage*)l_image, pixbuf);
      }
      gtk_window_resize(GTK_WINDOW(listener_window), l_bm->w, l_bm->h);
      gtk_window_set_position(GTK_WINDOW(listener_window), GTK_WIN_POS_CENTER_ALWAYS);
      gtk_widget_show(listener_window);
    }
    if (event->detail == 36) lenter_pressed = 1;
    if (event->detail == 9) lesc_pressed = 1;
    break;
  case XI_KeyRelease:
    printf("    flags: %s\n", (event->flags & XIKeyRepeat) ? "repeat" : "");
    if (event->detail == 133)
    {
      lmeta_pressed = 0;
      gtk_widget_hide(listener_window);
    }
    if (event->detail == 36) lenter_pressed = 0;
    if (event->detail == 9) lesc_pressed = 0;
    key_pressed = 1;
    break;
  }

  /* printf("    root: %.2f/%.2f\n", event->root_x, event->root_y); */
  /* printf("    event: %.2f/%.2f\n", event->event_x, event->event_y); */

  /* printf("    buttons:"); */
  /* for (i = 0; i < event->buttons.mask_len * 8; i++) */
  /*   if (XIMaskIsSet(event->buttons.mask, i)) */
  /*     printf(" %d", i); */
  /* printf("\n"); */

  /* printf("    modifiers: locked %#x latched %#x base %#x effective: %#x\n", */
  /*        event->mods.locked, event->mods.latched, */
  /*        event->mods.base, event->mods.effective); */
  /* printf("    group: locked %#x latched %#x base %#x effective: %#x\n", */
  /*        event->group.locked, event->group.latched, */
  /*        event->group.base, event->group.effective); */
  /* printf("    valuators:\n"); */

  /* val = event->valuators.values; */
  /* for (i = 0; i < event->valuators.mask_len * 8; i++) */
  /*   if (XIMaskIsSet(event->valuators.mask, i)) */
  /*     printf("        %i: %.2f\n", i, *val++); */

  /* printf("    windows: root 0x%lx event 0x%lx child 0x%lx\n", */
  /*        event->root, event->event, event->child); */
}

void* listener_start_thread()
{
  Display* display;

  XIEventMask mask;

  int xi_opcode;
  int event;
  int error;

  display = XOpenDisplay(NULL);

  if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error))
  {
    printf("X Input extension not available.\n");
  }

  Window win = DefaultRootWindow(display);

  mask.deviceid = XIAllDevices;
  mask.mask_len = XIMaskLen(XI_LASTEVENT);
  mask.mask     = calloc(mask.mask_len, sizeof(char));

  XISetMask(mask.mask, XI_KeyPress);
  XISetMask(mask.mask, XI_KeyRelease);

  XISelectEvents(display, win, &mask, 1);
  XSync(display, False);

  free(mask.mask);

  while (listener_alive)
  {
    XEvent               ev;
    XGenericEventCookie* cookie = (XGenericEventCookie*)&ev.xcookie;
    XNextEvent(display, (XEvent*)&ev);

    printf("LOOP\n");
    if (XGetEventData(display, cookie) &&
        cookie->type == GenericEvent &&
        cookie->extension == xi_opcode)
    {
      printf("EVENT type %d\n", cookie->evtype);
      print_deviceevent(cookie->data);
    }

    XFreeEventData(display, cookie);
  }

  XDestroyWindow(display, win);
  XSync(display, False);
  XCloseDisplay(display);

  return NULL;
}

void listener_start()
{
  pthread_t thread;
  pthread_create(&thread, NULL, (void*)listener_start_thread, NULL);
}

void listener_destroy()
{
  listener_alive = 0;
}

#endif
