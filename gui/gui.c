/*
 * $Id: gui.c,v 1.1 1996/07/11 15:37:26 grubba Exp $
 *
 * User Interface for the M68000 to Sparc recompiler
 *
 * $Log$
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


