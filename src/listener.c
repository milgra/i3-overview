#ifndef listener_h
#define listener_h

void listener_start(void (*show)(void), void (*hide)(void));
void listener_destroy();

#endif

#if __INCLUDE_LEVEL__ == 0

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int listener_alive = 1;
int meta_pressed   = 0;

void (*l_show)(void);
void (*l_hide)(void);

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

    if (XGetEventData(display, cookie) &&
        cookie->type == GenericEvent &&
        cookie->extension == xi_opcode)
    {

      XIDeviceEvent* event = cookie->data;

      // printf("    device: %d (%d)\n", event->deviceid, event->sourceid);
      // printf("    detail: %d\n", event->detail);

      switch (event->evtype)
      {
      case XI_KeyPress:
        if (event->detail == 133)
        {
          (*l_show)();
          meta_pressed = 1;
        }
        else if (meta_pressed)
        {
          (*l_show)();
        }
        break;
      case XI_KeyRelease:
        if (event->detail == 133)
        {
          (*l_hide)();
          meta_pressed = 0;
        }
        break;
      }
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
