#ifndef listener_h
#define listener_h
#include <gtk/gtk.h>

void listener_start(void (*show)(void), void (*hide)(void));
void listener_destroy();

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

void (*l_show)(void);
void (*l_hide)(void);

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
    if (event->detail == 133) (*l_show)();
    break;
  case XI_KeyRelease:
    printf("    flags: %s\n", (event->flags & XIKeyRepeat) ? "repeat" : "");
    if (event->detail == 133) (*l_hide)();
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

void listener_start(void (*show)(void), void (*hide)(void))
{
  l_show = show;
  l_hide = hide;
  pthread_t thread;
  pthread_create(&thread, NULL, (void*)listener_start_thread, NULL);
}

void listener_destroy()
{
  listener_alive = 0;
}

#endif
