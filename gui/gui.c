/*
 * $Id: gui.c,v 1.2 1996/07/11 20:13:01 grubba Exp $
 *
 * User Interface for the M68000 to Sparc recompiler
 *
 * $Log: gui.c,v $
 * Revision 1.1  1996/07/11 15:37:26  grubba
 * Graphics User-Interface files.
 * Initial version.
 *
 *
 */

/*
 * Includes
 */

#include <thread.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/xpm.h>

#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#include "../recomp.h"
#include "../m68k.h"
#include "../gui.h"

/*
 * Structures
 */

struct button_args {
  const char *label;
  void (*callback)(void *);
  void *callbackarg;
  int x, y;
  unsigned width, height;
  Window root;
  GC gc;
  XFontStruct *font;
  int backgroundPixel;
  int shadowPixel;
};

/*
 * Functions
 */

Window gui_CreateSimpleWindow(Display *display, Window root,
			      const char *window_name, const char *icon_name,
			      int x, int y, unsigned width, unsigned height,
			      unsigned border_width,
			      unsigned long border_color, unsigned long background)
{
  Window window;
  XTextProperty tp0, tp1;
  XSizeHints window_size;

  window = XCreateSimpleWindow(display, root, x, y, width, height,
			       border_width, border_color, background);
			       
  tp0.value = (unsigned char *)window_name;
  tp0.encoding = XA_STRING;
  tp0.format = 8;
  tp0.nitems = strlen(window_name);
  tp1.value = (unsigned char *)icon_name;
  tp1.encoding = XA_STRING;
  tp1.format = 8;
  tp1.nitems = strlen(icon_name);

  window_size.x = x;
  window_size.y = y;
  window_size.min_width = window_size.max_width = window_size.width = width;
  window_size.min_height = window_size.max_height = window_size.height = height;
  window_size.width_inc = window_size.height_inc = 0;
  window_size.win_gravity = CenterGravity;
  window_size.flags = USPosition | USSize | PPosition | PSize |
                      PMinSize | PMaxSize | PResizeInc | PWinGravity;

  XSetWMProperties(display, window, &tp0, &tp1, NULL, 0,
		   &window_size, NULL, NULL);

  return(window);
}

int gui_AllocColor(Display *display, Colormap cmap,
		   int red, int green, int blue, int fallback)
{
  XColor color;

  color.red = red * 0x0101;
  color.green = green * 0x0101;
  color.blue = blue * 0x0101;
  color.flags = DoRed|DoGreen|DoBlue;

  if (!XAllocColor(display, cmap, &color)) {
    return(fallback);
  } else {
    return(color.pixel);
  }
}

static void *gui_button_main(void *arg)
{
  struct button_args *args = (struct button_args *)arg;
  Display *display;
  
  if ((display = XOpenDisplay(NULL))) {
    int screen_num = DefaultScreen(display);
    Window window;
    XGCValues gcvalues;
    GC localgc;
    int xx, yy;
    
    window = XCreateSimpleWindow(display, args->root,
				 args->x, args->y, args->width, args->height, 0,
				 BlackPixel(display, screen_num),
				 args->backgroundPixel);

    localgc = XCreateGC(display, window, 0, &gcvalues);

    xx = (args->width - XTextWidth(args->font, args->label, strlen(args->label)))/2;
    yy = ((args->height - args->font->ascent - args->font->descent) / 2) +
         args->font->ascent;
    
    XSelectInput(display, window,
		 ExposureMask | ButtonPressMask | ButtonReleaseMask);

    XMapWindow(display, window);

    while (1) {
      XEvent event;

      XNextEvent(display, &event);

      switch(event.type) {
      case ButtonRelease:
	fprintf(stderr, "Button release: %s\n", args->label);

	args->callback(args->callbackarg);

	/* Fall through */
      case Expose:
	if ((event.type == Expose) && (event.xexpose.count)) {
	  break;
	}

	XSetForeground(display, localgc, WhitePixel(display, screen_num));
	XDrawLine(display, window, localgc, 0, args->height-2, 0, 0);
	XDrawLine(display, window, localgc, 0, 0, args->width-2, 0);
	XSetForeground(display, localgc, args->shadowPixel);
	XDrawLine(display, window, localgc,
		  1, args->height-2, args->width-2, args->height-2);
	XDrawLine(display, window, localgc,
		  args->width-2, args->height-2, args->width-2, 1);
	XSetForeground(display, localgc, BlackPixel(display, screen_num));
	XDrawLine(display, window, localgc,
		  0, args->height-1, args->width-1, args->height-1);
	XDrawLine(display, window, localgc,
		  args->width-1, args->height-1, args->width-1, 0);
	XDrawString(display, window, args->gc, xx, yy, args->label, strlen(args->label));
	break;
      case ButtonPress:
	break;
      }
    }
  }
  return (NULL);
}

void gui_AddButton(const char *label, void (*callback)(void *), void *callbackarg,
		   int x, int y, unsigned width, unsigned height,
		   Window window, GC gc, XFontStruct *font,
		   int backgroundPixel, int shadowPixel)
{
  struct button_args *args;

  if ((args = malloc(sizeof(struct button_args)))) {
    thread_t button_thread;

    args->label = label;
    args->callback = callback;
    args->callbackarg = callbackarg;
    args->x = x;
    args->y = y;
    args->width = width;
    args->height = height;
    args->root = window;
    args->gc = gc;
    args->font = font;
    args->backgroundPixel = backgroundPixel;
    args->shadowPixel = shadowPixel;

    thr_create(NULL, 0, gui_button_main, (void *)args,
	       THR_DETACHED | THR_DAEMON, &button_thread);
  }
}

struct my_x_string *new_my_x_string(const char *string, int x, int y,
				    GC gc, struct my_x_string *last_str)
{
  struct my_x_string *new_str;

  if ((new_str = calloc(sizeof(struct my_x_string), 1))) {
    new_str->next = last_str;
    new_str->x = x;
    new_str->y = y;
    new_str->gc = gc;
    new_str->string = string;
    new_str->str_len = strlen(string);
  }
  return (new_str);
}

GC my_create_font_gc(Display *display, int screen_num, Window window,
		     const char *font_name, XFontStruct **font_info)
{
  GC gc;
  XGCValues gcvalues;

  gc = XCreateGC(display, window, 0, &gcvalues);

  *font_info = XLoadQueryFont(display, font_name);

  XSetFont(display, gc, (*font_info)->fid);
  XSetForeground(display, gc, BlackPixel(display, screen_num));

  return (gc);
}


